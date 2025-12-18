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

void* kmalloc_aligned(size_t sz, uintptr_t* phys_addr) {
  if (bump_ptr &= 0xFFFFF000) {
    bump_ptr = (bump_ptr & 0xFFFFF000) + 0x1000;
  }

  if (phys_addr) {
    *phys_addr = bump_ptr;
  }

  uintptr_t addr = bump_ptr;
  bump_ptr += sz;
  return (void*)addr;
}

// vmm
page_directory_t kernel_directory __attribute__((aligned(4096)));
page_table_t first_table __attribute__((aligned(4096)));  // first 4mb

void paging_initialize(void) {
  // identity map the first 4mb
  page_table_t* table = &first_table;
  for (uint32_t i = 0; i < 1024; i++) {
    memset(&table->pages[i], 0, sizeof(page_t));
    table->pages[i].present = 1;
    table->pages[i].rw = 1;
    table->pages[i].frame = i;
  }

  // clear out everything first
  for (uint32_t i = 0; i < 1024; i++) {
    kernel_directory.tables[i] = 0;
    kernel_directory.physical_tables[i] = 0;
  }

  // set up the first 4mb table
  kernel_directory.tables[0] = &first_table;
  kernel_directory.physical_tables[0] =
      (uint32_t)&first_table | PG_PRESENT | PG_RW;

  // // toy attempt
  // // this should make 0xDEADBEEF point to frame 0x0
  // uintptr_t phys_addr_test = 0xDEADBEEF;
  // phys_addr_test /= 0x1000;
  // uint32_t table_idx = phys_addr_test / 1024;
  // uint32_t page_idx = phys_addr_test % 1024;
  // printk("[mem] 0xDEADBEEF table_idx=%d page_idx=%d \n", table_idx,
  // page_idx);

  // uint32_t test_table_phys_addr;
  // page_table_t* test_table = (page_table_t*)kmalloc_aligned(
  //     sizeof(page_table_t), (uintptr_t*)&test_table_phys_addr);

  // test_table->pages[page_idx].present = 1;
  // test_table->pages[page_idx].rw = 1;
  // test_table->pages[page_idx].frame = 0x0;

  // kernel_directory.tables[table_idx] = test_table;
  // kernel_directory.physical_tables[table_idx] =
  // test_table_phys_addr | PG_PRESENT | PG_RW;

  // lastly
  kernel_directory.physical_address = (uint32_t)&kernel_directory;
}

void paging_load_directory(page_directory_t* dir) {
  asm volatile("mov %0, %%cr3" ::"r"(dir->physical_address));
}

static void paging_enable(void) {
  uint32_t cr0;
  asm volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 |= 0x80000000;  // set the paging bit
  asm volatile("mov %0, %%cr0" ::"r"(cr0));
}

void paging_finalize(void) {
  paging_load_directory(&kernel_directory);

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