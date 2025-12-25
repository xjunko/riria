#include <riria/cpu/irq.h>
#include <riria/cpu/regs.h>
#include <riria/cpu/tss.h>
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
  for (;;)
    ;
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

int process_schedule(regs_t* r) {
  UNUSED(r);
  irq_ack(0);
  int_disable();

  if (!should_schedule || !process_head || !process_head->next) {
    int_resume();
    return 0;
  }

  process_t* prev_proc = process_head->process;
  ASSERT(prev_proc);
  process_head = process_head->next;
  ASSERT(process_head);
  process_t* curr_proc = process_head->process;
  ASSERT(curr_proc);

  ASSERT(curr_proc->pagemap);
  vmm_switch_pagemap(curr_proc->pagemap);

  ASSERT(curr_proc->stack_top);
  tss_set_stack((uint64_t)curr_proc->stack_top);

  ASSERT(&prev_proc->krsp);
  ASSERT(&curr_proc->krsp);
  process_switch(&prev_proc->krsp, &curr_proc->krsp);

  return 0;
}

process_t* process_get_current(void) {
  if (process_head) return process_head->process;
  return NULL;
}