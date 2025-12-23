#include <riria/cpu/isr.h>
#include <riria/cpu/regs.h>
#include <riria/libk.h>
#include <riria/mem.h>
#include <riria/types.h>

// pmm
extern void* end;
uintptr_t bump_ptr = (uintptr_t)&end;
uintptr_t heap_end = 0;

void kmalloc_start_at(uintptr_t addr) { bump_ptr = addr; }

void* kmalloc_real(size_t sz, int should_align, uintptr_t* phys_addr) {
  // heap
  if (heap_end) {
    void* addr;
    printk("[mem] kmalloc heap!");
    while (1) asm volatile("hlt");
  }

  // bump
  if (should_align && (bump_ptr & 0xFFFFF000)) {
    bump_ptr &= 0xFFFFF000;
    bump_ptr += 0x1000;
  }

  if (phys_addr) {
    *phys_addr = bump_ptr;
  }

  uintptr_t addr = bump_ptr;
  bump_ptr += sz;
  return (void*)addr;
}

void* kmalloc(size_t sz) { return kmalloc_real(sz, 0, NULL); }

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
page_directory_t* kernel_directory;

#define INDEX_FROM_BIT(x) (x / 0x20)
#define OFFSET_FROM_BIT(x) (x % 0x20)
#define MAX_ADDRESS 0xFFFFFFFF

uint32_t* frames;
uint32_t nframes;

/// sets the addr to used
void set_frame(uintptr_t addr) {
  if (addr < nframes * 0x1000) {
    uint32_t frame = addr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
  }
}

/// sets the addr to free
void clear_frame(uintptr_t addr) {
  if (addr < nframes * 0x1000) {
    uint32_t frame = addr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << off);
  }
}

/// returns 1 if addr is set, 0 if free
uint32_t test_frame(uintptr_t addr) {
  if (addr < nframes * 0x1000) {
    uint32_t frame = addr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    return (frames[idx] & (0x1 << off));
  }
  return 0;
}

/// returns the first free frame
uint32_t first_frame(void) {
  uint32_t i, j;

  for (i = 0; i < INDEX_FROM_BIT(nframes); i++) {
    if (frames[i] != MAX_ADDRESS) {
      for (j = 0; j < 32; j++) {
        uint32_t test_frame = (uint32_t)0x1 << j;
        if (!(frames[i] & test_frame)) {
          return i * 32 + j;
        }
      }
    }
  }

  return -1;
}

/// returns the first n contiguous free frames
uint32_t first_n_frames(int n) {
  for (uint32_t i = 0; i < nframes * 0x1000; i += 0x1000) {
    int bad = 0;
    for (int j = 0; j < n; j++) {
      if (test_frame(i + 0x1000 * j)) {
        bad = j + 1;
      }
    }

    if (!bad) {
      return i / 0x1000;
    }
  }

  return MAX_ADDRESS;
}

/// sets the page rw and user/kernel flags, allocates a frame if not yet
void alloc_frame(page_t* page, int is_kernel, int is_rw) {
  printk("[mem] alloc_frame\n");

  if (page->frame != 0) {
    page->present = 1;
    page->rw = (is_rw == 1) ? 1 : 0;
    page->user = (is_kernel == 1) ? 0 : 1;
  } else {
    uint32_t idx = first_frame();
    set_frame(idx * 0x1000);
    page->frame = idx;
    page->present = 1;
    page->rw = (is_rw == 1) ? 1 : 0;
    page->user = (is_kernel == 1) ? 0 : 1;
  }
}

/// gets the page for an address, creates one if `make` is set
page_t* page_get(uintptr_t addr, int make, page_directory_t* dir) {
  addr /= 0x1000;
  uint32_t table_idx = addr / 1024;
  if (dir->tables[table_idx]) {
    return &dir->tables[table_idx]->pages[addr % 1024];
  } else if (make) {
    printk("[mem] page_get addr=0x%x make=%d\n", addr, make);
    uint32_t temp_addr;  // using uin32_t here because a page is about 32 bit.
    dir->tables[table_idx] = (page_table_t*)kmalloc_aligned(
        sizeof(page_table_t), (uintptr_t*)&temp_addr);
    memset(dir->tables[table_idx], 0, sizeof(page_table_t));
    dir->physical_tables[table_idx] = temp_addr | PG_PRESENT | PG_RW | PG_USER;
    return &dir->tables[table_idx]->pages[addr % 1024];
  }

  return 0;
}

/// sets the page address and flags, also sets the `addr` to be used
/// internally.
void page_set_address(page_t* page, uintptr_t addr, int is_kernel, int is_rw) {
  page->present = 1;
  page->rw = (is_rw == 1) ? 1 : 0;
  page->user = (is_kernel == 1) ? 0 : 1;
  page->frame = addr / 0x1000;
  set_frame(addr);
}

/// sets the page's frame to nothing and clears the frame internally
void page_free(page_t* page) {
  if (page->frame) {
    uint32_t frame = page->frame;
    clear_frame(frame * 0x1000);
    page->frame = 0x0;
  } else {
    printk("[mem] PAGE_FREE: page not allocated\n");
    while (1) asm volatile("hlt");
  }
}

void paging_initialize(uint32_t mem_sz) {
  nframes = mem_sz / 4;
  frames = (uint32_t*)kmalloc(INDEX_FROM_BIT(nframes * 8));
  memset(frames, 0, INDEX_FROM_BIT(nframes * 8));

  kernel_directory =
      (page_directory_t*)kmalloc_aligned(sizeof(page_directory_t), 0);
  memset(kernel_directory, 0, sizeof(page_directory_t));
}

void paging_load_directory(page_directory_t* dir) {
  asm volatile("mov %0, %%cr3" ::"r"(dir->physical_address));
}

static void paging_enable(void) {
  // FIXME: 64bit conversion
  // uint32_t cr0;
  // asm volatile("mov %%cr0, %0" : "=r"(cr0));
  // cr0 |= 0x80000000;  // set the paging bit
  // asm volatile("mov %0, %%cr0" ::"r"(cr0));
}

void paging_finalize(void) {
  // set the null page to not present
  page_get(0, 1, kernel_directory)->present = 0;
  set_frame(0);

  // identity mapping the first 4MB
  for (uint32_t i = 0; i < 0x400000; i += 0x1000) {
    page_set_address(page_get(i, 1, kernel_directory), i, 1, 0);
  }

  // map the framebuffer, will only work on mb2 1280x720x32
  for (uint32_t i = 0xfe000000; i < 0xfe400000; i += 0x1000) {
    page_set_address(page_get(i, 1, kernel_directory), i, 1, 1);
  }

  kernel_directory->physical_address =
      (uintptr_t)kernel_directory->physical_tables;

  isr_install_handler(14, paging_fault);
  paging_load_directory(kernel_directory);
  paging_enable();

  // HACK
  heap_end = (bump_ptr + 0x1000) & ~0xFFF;
}

void paging_fault(regs_t* r) {
  // FIXME: 64bit conversion
  // printk("[mem] page fault at rip=0x%x ", r->rip);
  // uint32_t err_addr;
  // asm volatile("mov %%cr2, %0" : "=r"(err_addr));
  // printk("address=0x%x \n", err_addr);

  // // TODO: optimally we should handler some edge case here.

  // int present = !(r->err_code & 0x1) ? 1 : 0;
  // int rw = (r->err_code & 0x2) ? 1 : 0;
  // int user = (r->err_code & 0x4) ? 1 : 0;
  // int reserved = (r->err_code & 0x8) ? 1 : 0;
  // int id = (r->err_code & 0x10) ? 1 : 0;

  // printk("[kernel] segmentation fault\n");
  // printk("    present=%d rw=%d user=%d reserved=%d id=%d\n", present, rw,
  // user,
  //        reserved, id);

  while (1) asm volatile("hlt");
}