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
#include <riria/mem.h>
#include <riria/types.h>
#include <stdio.h>
#include <string.h>

extern uint8_t _text_begin[], _bss_end[];

#define PMM_BITMAP_SIZE (1024 * 1024)
#define BITMAP_GET(index) (bitmap[(index) / 8] & (1 << ((index) % 8)))
#define BITMAP_SET(index) (bitmap[(index) / 8] |= (1 << ((index) % 8)))
#define BITMAP_CLEAR(index) (bitmap[(index) / 8] &= ~(1 << ((index) % 8)))

static uint8_t bitmap[PMM_BITMAP_SIZE];

static size_t highest_page = 0;
static size_t used_pages = 0;
static size_t free_pages = 0;
static size_t total_ram_pages = 0;

void pmm_install(void) {
  printf("[pmm] PMM INIT...");

  size_t entry_count = memmap_request.response->entry_count;
  struct limine_memmap_entry** entries = memmap_request.response->entries;
  uintptr_t highest_address = 0;
  for (size_t i = 0; i < entry_count; i++) {
    struct limine_memmap_entry* entry = entries[i];
    uintptr_t top = entry->base + entry->length;
    if (top > highest_address) {
      highest_address = top;
    }
  }
  printf(", highest_address=0x%lx", highest_address);

  highest_page = (highest_address - 1) / PAGE_SIZE;
  size_t max_bitmap_pages = PMM_BITMAP_SIZE * 8;
  if (highest_page >= max_bitmap_pages) {
    printf(", limiting highest_page from %lu to max supported %lu pages",
           highest_page, max_bitmap_pages);
    highest_page = max_bitmap_pages - 1;
  }
  memset(bitmap, 0xFF, PMM_BITMAP_SIZE);

  total_ram_pages = 0;
  for (size_t i = 0; i < entry_count; i++) {
    struct limine_memmap_entry* entry = entries[i];

    if (entry->type == LIMINE_MEMMAP_USABLE ||
        entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
      uintptr_t base = ALIGN_UP(entry->base, PAGE_SIZE);
      uintptr_t top = (entry->base + entry->length) & ~(PAGE_SIZE - 1);

      if (top <= base) continue;

      size_t first_page = base / PAGE_SIZE;
      size_t last_page = top / PAGE_SIZE;

      for (size_t page_idx = first_page; page_idx < last_page; ++page_idx) {
        if (page_idx <= highest_page) {
          if (BITMAP_GET(page_idx)) {
            BITMAP_CLEAR(page_idx);
            total_ram_pages++;
          }
        }
      }
    }
  }

  free_pages = total_ram_pages;
  printf(", total_ram=%luMiB", (total_ram_pages * PAGE_SIZE) / (1024 * 1024));

  struct limine_executable_address_response* kaddr =
      executable_address_request.response;
  uintptr_t k_phys_start = kaddr->physical_base;
  uintptr_t k_virt_start = (uintptr_t)_text_begin;
  uintptr_t k_virt_end = (uintptr_t)_bss_end;
  uintptr_t k_size = k_virt_end - k_virt_start;
  uintptr_t k_phys_end = k_phys_start + ALIGN_UP(k_size, PAGE_SIZE);

  size_t k_first_page = k_phys_start / PAGE_SIZE;
  size_t k_last_page = k_phys_end / PAGE_SIZE;

  for (size_t page_idx = k_first_page; page_idx < k_last_page; ++page_idx) {
    if (page_idx <= highest_page) {
      if (!BITMAP_GET(page_idx)) {
        free_pages--;
      }
      BITMAP_SET(page_idx);
    }
  }

  uintptr_t bitmap_virt_start = (uintptr_t)bitmap;
  uintptr_t bitmap_phys_start =
      bitmap_virt_start - kaddr->virtual_base + kaddr->physical_base;
  uintptr_t bitmap_phys_end = bitmap_phys_start + PMM_BITMAP_SIZE;
  size_t bitmap_first_page = bitmap_phys_start / PAGE_SIZE;
  size_t bitmap_last_page = (bitmap_phys_end + PAGE_SIZE - 1) / PAGE_SIZE;

  for (size_t page_idx = bitmap_first_page; page_idx < bitmap_last_page;
       ++page_idx) {
    if (page_idx <= highest_page) {
      if (!BITMAP_GET(page_idx)) {
        free_pages--;
      }
      BITMAP_SET(page_idx);
    }
  }

  size_t pages_below_1mb = 0x100000 / PAGE_SIZE;
  for (size_t page_idx = 0; page_idx < pages_below_1mb; ++page_idx) {
    if (page_idx <= highest_page) {
      if (!BITMAP_GET(page_idx)) {
        free_pages--;
      }
      BITMAP_SET(page_idx);
    }
  }

  used_pages = total_ram_pages - free_pages;
  printf(", OK!\n");
}

void* pmm_allocate(void) {
  int_disable();
  for (size_t i = 0; i <= highest_page; ++i) {
    if (!BITMAP_GET(i)) {
      BITMAP_SET(i);
      used_pages++;
      free_pages--;

      void* phys = (void*)((uintptr_t)i * PAGE_SIZE);
      void* vaddr = (void*)((uintptr_t)phys + VMM_HIGHER_HALF);

      memset(vaddr, 0, PAGE_SIZE);
      int_enable();

      return phys;
    }
  }
  int_enable();
  panic("out of memory in pmm_allocate!");
  return NULL;
}

void pmm_free(void* page) {
  if (page == NULL) {
    panic("pmm_free called with NULL pointer!");
  }

  uintptr_t phys_addr = (uintptr_t)page;
  if (phys_addr % PAGE_SIZE != 0) {
    panic("pmm_free called with unaligned address!");
  }

  size_t idx = phys_addr / PAGE_SIZE;
  if (idx > highest_page) {
    panic("pmm_free called with out-of-bounds address!");
  }

  if (!BITMAP_GET(idx)) {
    panic("pmm_free called on already free page!");
  }

  BITMAP_CLEAR(idx);
  used_pages--;
  free_pages++;
}