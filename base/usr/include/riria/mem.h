#pragma once
#include <riria/cpu/regs.h>
#include <stddef.h>
#include <stdint.h>

// pmm
void kmalloc_start_at(uintptr_t);

void* kmalloc(size_t);
void* kmalloc_real(size_t, int, uintptr_t*);
void* kmalloc_aligned(size_t, uintptr_t*);

// vmm
#define PG_PRESENT 0x1
#define PG_RW 0x2
#define PG_USER 0x4

typedef struct page {
  unsigned int present : 1;
  unsigned int rw : 1;
  unsigned int user : 1;
  unsigned int writethrough : 1;
  unsigned int cachedisable : 1;
  unsigned int accessed : 1;
  unsigned int dirty : 1;
  unsigned int pat : 1;
  unsigned int global : 1;
  unsigned int unused : 3;
  unsigned int frame : 20;
} __attribute__((packed)) page_t;

typedef struct page_table {
  page_t pages[1024];
} page_table_t;

typedef struct page_directory {
  uintptr_t physical_tables[1024];
  page_table_t* tables[1024];
  uintptr_t physical_address;
} page_directory_t;

extern page_directory_t* kernel_directory;

// bitmap
void set_frame(uintptr_t);
void clear_frame(uintptr_t);
uint32_t test_frame(uintptr_t);
uint32_t first_frame(void);
uint32_t first_n_frames(int);
void alloc_frame(page_t*, int, int);

void page_set_address(page_t*, uintptr_t, int, int);
void page_free(page_t*);
page_t* page_get(uintptr_t, int, page_directory_t*);

void paging_initialize(uint32_t);
void paging_load_directory(page_directory_t*);
void paging_finalize(void);

void paging_fault(regs_t*);