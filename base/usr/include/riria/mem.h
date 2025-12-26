#pragma once
#include <riria/boot.h>
#include <riria/cpu/regs.h>

// heap
typedef struct heap_block {
  size_t size;
  struct heap_block* next;
} heap_block_t;

#define MIN_ALLOC_SIZE sizeof(heap_block_t)
#define HEAP_ALIGNMENT 16

#define ALIGN_UP_HEAP(size) \
  (((size) + HEAP_ALIGNMENT - 1) & ~(HEAP_ALIGNMENT - 1))

void heap_install(void);
int heap_expand_pages(size_t);

void* kmalloc(size_t);
void* kmalloc_phys(size_t, uintptr_t*);
void kfree(void*);

// pmm
void pmm_install(void);
void* pmm_allocate(void);
void pmm_free(void*);

// vmm
#define PAGE_SIZE 4096

#define PTE_PRESENT (1ull << 0)
#define PTE_WRITABLE (1ull << 1)
#define PTE_USER (1ull << 2)
#define PTE_PWT (1ull << 3)
#define PTE_PCD (1ull << 4)
#define PTE_ACCESSED (1ull << 5)
#define PTE_DIRTY (1ull << 6)
#define PTE_PAT (1ull << 7)
#define PTE_GLOBAL (1ull << 8)
#define PTE_NX (1ull << 63)

#define PTE_DUMMY (1ull << 9)

#define PTE_ADDR_MASK 0x000ffffffffff000
#define PTE_GET_ADDR(VALUE) ((VALUE)&PTE_ADDR_MASK)
#define PTE_GET_FLAGS(VALUE) ((VALUE) & ~PTE_ADDR_MASK)

#define ALIGN_UP(value, align) (((value) + (align)-1) & ~((align)-1))

#define VMM_HIGHER_HALF (hhdm_request.response->offset)

typedef struct pagemap {
  uint64_t* base_virt;
} pagemap_t;

extern pagemap_t* kernel_pagemap;

void vmm_install(void);
pagemap_t* vmm_new_pagemap(void);
void vmm_switch_pagemap(pagemap_t*);
void vmm_invalidate_page(uintptr_t);
uint64_t* vmm_get_next_level(uint64_t*, size_t, bool, uint64_t);
uintptr_t vmm_virt_to_phys(pagemap_t*, uintptr_t);
bool vmm_map_page(pagemap_t*, uintptr_t, uintptr_t, uint64_t);
bool vmm_unmap_page(pagemap_t*, uintptr_t);

void vmm_pagefault(regs_t* r);