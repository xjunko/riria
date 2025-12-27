#include <riria/cpu/irq.h>
#include <riria/cpu/msr.h>
#include <riria/cpu/regs.h>
#include <riria/cpu/tss.h>
#include <riria/elf.h>
#include <riria/mem.h>
#include <riria/process.h>
#include <riria/types.h>
#include <stdlib.h>
#include <string.h>

process_node_t* process_head = NULL;
uint32_t next_pid = 0;
bool should_schedule = false;

#define PUSH_STACK(stack, value) *--stack = value;

void _null(void) {
  process_get_current()->state = PROCESS_DEAD;
  HALT();
}

void process_create(process_entry_t entry, pagemap_t* pagemap) {
  printf("[prc] new process (entry=%p) (pagemap=%p)\n", entry, pagemap);

  process_t* new_process = malloc(sizeof(process_t));
  ASSERT(new_process);
  memset(new_process, 0, sizeof(process_t));

  new_process->stack = malloc(STACK_SIZE);
  ASSERT(new_process->stack);
  memset(new_process->stack, 0, STACK_SIZE);

  new_process->stack_top =
      (void*)(((uintptr_t)new_process->stack + STACK_SIZE) & ~0xF);

  new_process->is_user = false;
  new_process->id = next_pid++;

  if (!pagemap) {
    new_process->pagemap = kernel_pagemap;
  } else {
    new_process->pagemap = pagemap;
  }
  printf("[prc]  PGM=%p\n", new_process->pagemap);
  printf("[prc]  PID=%u\n", new_process->id);

  uint64_t* stack = (uint64_t*)new_process->stack_top;

  if (!entry) {
    entry = _null;
  }

  PUSH_STACK(stack, (uint64_t)_null);
  PUSH_STACK(stack, (uint64_t)entry);
  PUSH_STACK(stack, 0x202);

  PUSH_STACK(stack, 0);
  PUSH_STACK(stack, 0);
  PUSH_STACK(stack, 0);
  PUSH_STACK(stack, 0);
  PUSH_STACK(stack, 0);
  PUSH_STACK(stack, 0);

  new_process->krsp = (uint64_t)stack;
  new_process->ursp = 0;

  process_node_t* new_node = malloc(sizeof(process_node_t));
  new_node->process = new_process;

  if (!process_head) {
    new_node->next = new_node;
    process_head = new_node;
  } else {
    new_node->next = process_head->next;
    process_head->next = new_node;
  }

  should_schedule = true;
}

// clang-format off
__attribute__((noreturn))
// used like: process_spawn_user(buf, sz, USER_VIRT_START);
// clang-format on
void process_spawn_user(const uint8_t* code, size_t len, uint64_t entry_addr) {
  UNUSED(len);
  uint64_t flags = PTE_PRESENT | PTE_WRITABLE | PTE_USER;
  uintptr_t virt_start = entry_addr;
  size_t mem_required = len;
  uintptr_t virt_end =
      (entry_addr + PAGE_SIZE + mem_required - 1) & ~(PAGE_SIZE - 1);

  for (size_t i = virt_start; i < virt_end; i += PAGE_SIZE) {
    uintptr_t page = (uintptr_t)pmm_allocate();
    vmm_map_page(process_get_current()->pagemap, i, page, flags);
  }

  uintptr_t stack_top = USER_STACK_TOP;
  uintptr_t stack_base = USER_STACK_BASE;

  for (uintptr_t i = stack_base; i < stack_top; i += PAGE_SIZE) {
    uintptr_t page = (uintptr_t)pmm_allocate();
    vmm_map_page(process_get_current()->pagemap, i, page, flags);
  }

  memcpy((void*)entry_addr, code, len);

  asm volatile("swapgs");
  switch_to_user(entry_addr);
}

// clang-format off
__attribute__((noreturn))
// clang-format on
void process_spawn_elf(uint8_t* elf_data, size_t len) {
  uint64_t flags = PTE_PRESENT | PTE_WRITABLE | PTE_USER;
  uint64_t entry_addr = elf64_load(elf_data, len);

  uintptr_t stack_top = USER_STACK_TOP;
  uintptr_t stack_base = USER_STACK_BASE;

  for (uintptr_t i = stack_base; i < stack_top; i += PAGE_SIZE) {
    uintptr_t page = (uintptr_t)pmm_allocate();
    vmm_map_page(process_get_current()->pagemap, i, page, flags);
  }

  asm volatile("swapgs");
  switch_to_user(entry_addr);
}

void process_reap(void) {
  if (!process_head) return;
  if (!process_head->next) return;

  process_node_t* prev = process_head;
  process_node_t* curr = process_head->next;

  do {
    process_t* proc = curr->process;
    if (proc->state == PROCESS_DEAD) {
      printf("[prc] reaping process %u\n", proc->id);

      if (curr == prev) {
        process_head = NULL;
      } else {
        prev->next = curr->next;
        if (process_head == curr) {
          process_head = prev;
        }
      }

      // unmap the stacks
      uintptr_t stack_top = USER_STACK_TOP;
      uintptr_t stack_base = USER_STACK_BASE;
      printf("[prc] unmapping user stack pages...");
      for (uintptr_t i = stack_base; i < stack_top; i += PAGE_SIZE) {
        vmm_unmap_page(proc->pagemap, i);
      }
      printf(" OK!\n");

      free(proc->stack);
      free(proc);
      free(curr);

      curr = prev->next;
    } else {
      prev = curr;
      curr = curr->next;
    }
  } while (curr != process_head && process_head != NULL);
}

// accepts regs_t but dont need it.
int process_schedule(regs_t* r) {
  UNUSED(r);

  irq_ack(0);
  int_disable();
  irq_ack(0);

  process_reap();

  if (!should_schedule || !process_head || !process_head->next) goto cleanup;
  process_reap();

  process_t* prev_proc = process_head->process;
  ASSERT(prev_proc);
  process_head = process_head->next;
  ASSERT(process_head);
  process_t* curr_proc = process_head->process;
  ASSERT(curr_proc);

  // o.O
  if (curr_proc->id == prev_proc->id) goto cleanup;

  ASSERT(curr_proc->pagemap);
  vmm_switch_pagemap(curr_proc->pagemap);

  ASSERT(curr_proc->stack_top);
  tss_set_stack((uint64_t)curr_proc->stack_top);
  wrmsr(MSR_USER_GS_BASE, (uint64_t)curr_proc);

  ASSERT(&prev_proc->krsp);
  ASSERT(&curr_proc->krsp);
  process_switch(&prev_proc->krsp, &curr_proc->krsp);

cleanup:
  int_resume();
  return 0;
}

void process_exit(int code) {
  UNUSED(code);
  if (!process_get_current()) {
    panic("exiting without a process");
  }
  process_get_current()->state = PROCESS_DEAD;
  process_schedule(NULL);
}

// there should be a process, always.
process_t* process_get_current(void) {
  if (process_head) return process_head->process;
  UNREACHABLE();
}