/*
 * Arikoto
 * Copyright (c) 2025
 * Licensed under the NCSA/University of Illinois Open Source License; see the
 * following licence text
 *
 * NCSA/University of Illinois Open Source License
 *
 * Copyright (c) 2025 NerdNextDoor
 * All rights reserved.
 *
 * Developed by: Arikoto Operating System Development Project
 * https://arikoto.nerdnextdoor.net, https://codeberg.org/NerdNextDoor/arikoto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * with the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimers. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimers in the documentation and/or other
 * materials provided with the distribution. Neither the names of the Arikoto
 * Operating System Development Project, NerdNextDoor, nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * Software without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH
 * THE SOFTWARE.
 */

#include <riria/boot.h>
#include <riria/cpu/irq.h>
#include <riria/cpu/isr.h>
#include <riria/cpu/regs.h>
#include <riria/mem.h>
#include <riria/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern uint8_t _text_begin[], _text_end[];
extern uint8_t _rodata_begin[], _rodata_end[];
extern uint8_t _data_begin[], _data_end[];
extern uint8_t _bss_begin[], _bss_end[];

pagemap_t* kernel_pagemap = NULL;

static bool vmm_is_table_empty(uint64_t* table) {
  for (int i = 0; i < 512; i++) {
    if (table[i] != 0) {
      return false;
    }
  }

  return true;
}

pagemap_t* vmm_new_pagemap(void) {
  void* pml4_phys = pmm_allocate();
  if (!pml4_phys) {
    panic("failed to allocate new pagemap pml4");
  }

  uint64_t* pml4_virt = (uint64_t*)((uintptr_t)pml4_phys + VMM_HIGHER_HALF);
  memcpy(pml4_virt, kernel_pagemap->base_virt, PAGE_SIZE);
  memset(pml4_virt, 0, PAGE_SIZE / 2);

  pagemap_t* new_pagemap = malloc(sizeof(pagemap_t));
  new_pagemap->base_virt = pml4_virt;

  return new_pagemap;
}

void vmm_switch_pagemap(pagemap_t* pagemap) {
  if (!pagemap || !pagemap->base_virt) {
    panic("invalid pagemap while switching");
  }

  uintptr_t pml4_phys = (uintptr_t)pagemap->base_virt - VMM_HIGHER_HALF;
  asm volatile("mov %0, %%cr3" ::"r"(pml4_phys) : "memory");
}

void vmm_invalidate_page(uintptr_t addr) {
  asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

uint64_t* vmm_get_next_level(uint64_t* current_level_virt, size_t idx,
                             bool allocate, uint64_t alloc_entry_flags) {
  uint64_t entry = current_level_virt[idx];

  if (entry & PTE_PRESENT) {
    return (uint64_t*)(PTE_GET_ADDR(entry) + VMM_HIGHER_HALF);
  }

  if (!allocate) return NULL;

  void* next_level_phys = pmm_allocate();
  if (!next_level_phys) {
    panic("failed to allocate page for vmm_get_next_level");
  }

  uint64_t* next_level_virt =
      (uint64_t*)((uintptr_t)next_level_phys + VMM_HIGHER_HALF);
  memset(next_level_virt, 0, PAGE_SIZE);

  uint64_t new_entry_flags =
      alloc_entry_flags ? alloc_entry_flags : (PTE_PRESENT | PTE_WRITABLE);
  current_level_virt[idx] =
      (uint64_t)(uintptr_t)next_level_phys | new_entry_flags;

  return next_level_virt;
}

bool vmm_map_page(pagemap_t* pagemap, uintptr_t virt, uintptr_t phys,
                  uint64_t flags) {
  int_disable();

  virt &= ~(PAGE_SIZE - 1);
  phys &= ~(PAGE_SIZE - 1);

  size_t pml4_idx = (virt >> 39) & 0x1FF;
  size_t pdpt_idx = (virt >> 30) & 0x1FF;
  size_t pd_idx = (virt >> 21) & 0x1FF;
  size_t pt_idx = (virt >> 12) & 0x1FF;

  uint64_t alloc_flags = PTE_PRESENT | PTE_WRITABLE;
  if (flags & PTE_USER) alloc_flags |= PTE_USER;

  uint64_t* pml4 = pagemap->base_virt;
  if (!pml4) goto fail;
  uint64_t* pdpt = vmm_get_next_level(pml4, pml4_idx, true, alloc_flags);
  if (!pdpt) goto fail;
  uint64_t* pd = vmm_get_next_level(pdpt, pdpt_idx, true, alloc_flags);
  if (!pd) goto fail;
  uint64_t* pt = vmm_get_next_level(pd, pd_idx, true, alloc_flags);
  if (!pt) goto fail;

  pt[pt_idx] = phys | flags | PTE_PRESENT;
  vmm_invalidate_page(virt);

  int_enable();
  return true;

fail:
  printf("failed to map page for virt=0x%x", virt);
  panic("vmm_map_page failure");
  return false;
}

bool vmm_unmap_page(pagemap_t* pagemap, uintptr_t virt, uintptr_t phys,
                    uint64_t flags) {
  UNUSED(phys);
  UNUSED(flags);

  if (virt % PAGE_SIZE != 0) {
    panic("page not aligned in vmm_unmap_page");
  }

  size_t pml4_idx = (virt >> 39) & 0x1FF;
  size_t pdpt_idx = (virt >> 30) & 0x1FF;
  size_t pd_idx = (virt >> 21) & 0x1FF;
  size_t pt_idx = (virt >> 12) & 0x1FF;

  uint64_t* pml4 = pagemap->base_virt;
  uint64_t* pdpt = vmm_get_next_level(pml4, pml4_idx, false, 0);
  if (!pdpt) {
    return true;
  }
  uint64_t pdpt_entry = pml4[pml4_idx];

  uint64_t* pd = vmm_get_next_level(pdpt, pdpt_idx, false, 0);
  if (!pd) {
    return true;
  }
  uint64_t pd_entry = pdpt[pdpt_idx];
  uint64_t* pt = vmm_get_next_level(pd, pd_idx, false, 0);
  if (!pt) {
    return true;
  }
  uint64_t pt_entry = pd[pd_idx];

  if (!(pt[pt_idx] & PTE_PRESENT)) {
    return true;
  }

  // null out the page table entry
  pt[pt_idx] = 0;
  vmm_invalidate_page(virt);

  // if pt is empty, empty it
  if (vmm_is_table_empty(pt)) {
    pd[pd_idx] = 0;

    uintptr_t pt_phys = PTE_GET_ADDR(pt_entry);
    pmm_free((void*)pt_phys);

    // if pd is empty, empty it
    if (vmm_is_table_empty(pd)) {
      pdpt[pdpt_idx] = 0;

      uintptr_t pd_phys = PTE_GET_ADDR(pd_entry);
      pmm_free((void*)pd_phys);

      // if pdpt is empty, empty it
      if (vmm_is_table_empty(pdpt)) {
        if (pml4_idx >= 256) {
          return true;
        }
        pml4[pml4_idx] = 0;
        uintptr_t pdpt_phys = PTE_GET_ADDR(pdpt_entry);
        pmm_free((void*)pdpt_phys);
      }
    }
  }

  return true;
}

uintptr_t vmm_virt_to_phys(pagemap_t* pagemap, uintptr_t virt) {
  size_t pml4_idx = (virt >> 39) & 0x1FF;
  size_t pdpt_idx = (virt >> 30) & 0x1FF;
  size_t pd_idx = (virt >> 21) & 0x1FF;
  size_t pt_idx = (virt >> 12) & 0x1FF;

  uint64_t* pml4 = pagemap->base_virt;
  uint64_t* pdpt = vmm_get_next_level(pml4, pml4_idx, false, 0);
  if (pdpt == NULL) return (uintptr_t)-1;
  uint64_t* pd = vmm_get_next_level(pdpt, pdpt_idx, false, 0);
  if (pd == NULL) return (uintptr_t)-1;
  uint64_t* pt = vmm_get_next_level(pd, pd_idx, false, 0);
  if (pt == NULL) return (uintptr_t)-1;

  uint64_t entry = pt[pt_idx];

  if (!(entry & PTE_PRESENT)) return (uintptr_t)-1;

  uintptr_t phys = PTE_GET_ADDR(entry);
  uintptr_t offset = virt % PAGE_SIZE;
  return phys + offset;
}

void vmm_install(void) {
  printf("[vmm] VMM INIT...");
  void* pml4_phys = pmm_allocate();

  if (!pml4_phys) {
    panic("failed to allocate kernel pml4");
  }

  uint64_t* pml4_virt = (uint64_t*)((uintptr_t)pml4_phys + VMM_HIGHER_HALF);
  memset(pml4_virt, 0, PAGE_SIZE);

  static pagemap_t new_pagemap;
  kernel_pagemap = &new_pagemap;
  kernel_pagemap->base_virt = pml4_virt;

  for (int i = 0; i < 512; i++) {
    vmm_get_next_level(pml4_virt, i, true, PTE_DUMMY);
  }

  struct limine_executable_address_response* kaddr =
      executable_address_request.response;
  uintptr_t kernel_phys_base = kaddr->physical_base;
  uintptr_t kernel_virt_base = kaddr->virtual_base;

  uintptr_t text_start_addr = (uintptr_t)_text_begin;
  uintptr_t text_end_addr = (uintptr_t)_text_end;
  uintptr_t rodata_start_addr = (uintptr_t)_rodata_begin;
  uintptr_t rodata_end_addr = (uintptr_t)_rodata_end;
  uintptr_t data_start_addr = (uintptr_t)_data_begin;
  uintptr_t data_end_addr = ALIGN_UP((uintptr_t)_bss_end, PAGE_SIZE);

  uintptr_t kernel_virt_end = data_end_addr;

  for (uintptr_t p_virt = kernel_virt_base; p_virt < kernel_virt_end;
       p_virt += PAGE_SIZE) {
    uintptr_t p_phys = (p_virt - kernel_virt_base) + kernel_phys_base;
    uint64_t flags = PTE_PRESENT;

    if (p_virt >= text_start_addr && p_virt < text_end_addr) {
      // noop
    } else if (p_virt >= rodata_start_addr && p_virt < rodata_end_addr) {
      flags |= PTE_NX;
    } else if (p_virt >= data_start_addr && p_virt < data_end_addr) {
      flags |= PTE_WRITABLE | PTE_NX;
    } else {
      flags |= PTE_WRITABLE | PTE_NX;
    }

    if (!vmm_map_page(kernel_pagemap, p_virt, p_phys, flags)) {
      panic("failed to map kernel page");
    }
  }

  struct limine_memmap_response* memmap = memmap_request.response;
  for (size_t i = 0; i < memmap->entry_count; i++) {
    struct limine_memmap_entry* entry = memmap->entries[i];

    uintptr_t base = entry->base;
    uintptr_t top = base + entry->length;
    uintptr_t map_base = ALIGN_UP(base, PAGE_SIZE);
    uintptr_t map_top = top & ~(PAGE_SIZE - 1);

    if (map_top <= map_base) continue;

    for (uintptr_t p = map_base; p < map_top; p += PAGE_SIZE) {
      if (!vmm_map_page(kernel_pagemap, p + VMM_HIGHER_HALF, p,
                        PTE_PRESENT | PTE_WRITABLE | PTE_NX)) {
        panic("failed to map hhdm page");
      }
    }

    const uintptr_t ident_map_limit = 0x00100000;  // 1MB
    if (map_base < ident_map_limit) {
      uintptr_t ident_map_end =
          (map_top > ident_map_limit) ? ident_map_limit : map_top;

      for (uintptr_t p = map_base; p < ident_map_end; p += PAGE_SIZE) {
        if (!vmm_map_page(kernel_pagemap, p, p,
                          PTE_PRESENT | PTE_WRITABLE | PTE_NX)) {
          panic("failed to identity map low 1mb page");
        }
      }
    }
  }

  vmm_switch_pagemap(kernel_pagemap);
  printf(" OK!\n");

  // page fault handler
  isr_install_handler(14, vmm_pagefault);
}

void vmm_pagefault(regs_t* r) {
  // print the registers and panic
  uint64_t fault_addr;
  asm volatile("mov %%cr2, %0" : "=r"(fault_addr));

  printf("[err] page fault at address 0x%x\n", fault_addr);
  print_regs(r);

  panic("erm, page fault occurred");
}