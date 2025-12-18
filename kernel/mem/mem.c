#include <riria/cpu/isr.h>
#include <riria/cpu/regs.h>
#include <riria/libk.h>
#include <riria/mem.h>
#include <stddef.h>
#include <stdint.h>

// pmm
extern void* end;
uintptr_t bump_ptr = (uintptr_t)&end;

void kmalloc_start_at(uintptr_t addr) { bump_ptr = addr; }

void* kmalloc(size_t sz) {
  uintptr_t addr = bump_ptr;
  bump_ptr += sz;
  return (void*)addr;
}

// vmm
uint32_t page_dir[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));  // first 4mb

void paging_initialize(void) {
  // identity map the first 4mb
  for (uint32_t i = 0; i < 1024; i++) {
    first_page_table[i] = (i * 0x1000) | PG_PRESENT | PG_RW;
  }

  // make sure page_dir is init to 0
  // FIXME: can probably use memset for this?
  for (uint32_t i = 0; i < 1024; i++) {
    page_dir[i] = 0;
  }

  page_dir[0] = ((uint32_t)first_page_table) | PG_PRESENT | PG_RW;
}

void paging_load_directory(uint32_t dir) {
  asm volatile("mov %0, %%cr3" ::"r"(dir));
}

static void paging_enable(void) {
  uint32_t cr0;
  asm volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 |= 0x80000000;  // set the paging bit
  asm volatile("mov %0, %%cr0" ::"r"(cr0));
}

void paging_finalize(void) {
  paging_load_directory((uint32_t)page_dir);

  // TODO: some more stuff
  isr_install_handler(14, paging_fault);

  paging_enable();
}

void paging_fault(regs_t* r) {
  printk("[mem] page fault at eip=0x%x ", r->eip);
  uint32_t err_addr;
  asm volatile("mov %%cr2, %0" : "=r"(err_addr));
  printk("address=0x%x \n", err_addr);

  // TODO: optimally we should handler some edge case here.

  int present = !(r->err_code & 0x1) ? 1 : 0;
  int rw = (r->err_code & 0x2) ? 1 : 0;
  int user = (r->err_code & 0x4) ? 1 : 0;
  int reserved = (r->err_code & 0x8) ? 1 : 0;
  int id = (r->err_code & 0x10) ? 1 : 0;

  printk("[kernel] segmentation fault\n");
  printk("    present=%d rw=%d user=%d reserved=%d id=%d\n", present, rw, user,
         reserved, id);

  while (1) asm volatile("hlt");
}