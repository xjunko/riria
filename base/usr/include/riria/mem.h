#pragma once
#include <riria/cpu/regs.h>
#include <stddef.h>
#include <stdint.h>

// pmm
void kmalloc_start_at(uintptr_t);
void* kmalloc(size_t);

// vmm
#define PG_PRESENT 0x1
#define PG_RW 0x2

void paging_initialize(void);
void paging_load_directory(uint32_t);
void paging_finalize(void);

void paging_fault(regs_t*);