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
#include <riria/mem.h>
#include <riria/types.h>
#include <stdio.h>

#define KERNEL_HEAP_START 0xFFFF810000000000
#define KERNEL_INITIAL_PAGES 256

static void* heap_start = NULL;
static size_t heap_size = 0;
static heap_block_t* heap_head = NULL;

int heap_expand_pages(size_t pages) {
  if (pages == 0) return 0;

  uintptr_t base = (uintptr_t)heap_start + heap_size;
  uintptr_t new_region_size = pages * PAGE_SIZE;

  for (size_t i = 0; i < pages; ++i) {
    void* phys = pmm_allocate();
    if (!phys) {
      panic("heap: failed to allocate page for heap expansion");
    }

    uintptr_t virt = base + (i * PAGE_SIZE);
    if (!vmm_map_page(kernel_pagemap, virt, (uintptr_t)phys,
                      PTE_PRESENT | PTE_WRITABLE | PTE_NX)) {
      panic("heap: vmm_map_page failed during heap expansion");
    }
  }

  heap_block_t* new_block = (heap_block_t*)base;
  new_block->size = new_region_size;
  new_block->next = NULL;

  if (!heap_head) {
    heap_head = new_block;
  } else {
    heap_block_t* prev = NULL;
    heap_block_t* curr = heap_head;

    while (curr && (uintptr_t)curr < base) {
      prev = curr;
      curr = curr->next;
    }

    if (prev == NULL) {
      new_block->next = heap_head;
      heap_head = new_block;
    } else {
      new_block->next = prev->next;
      prev->next = new_block;
    }

    // merge adjacent blocks
    if (new_block->next && ((uintptr_t)new_block + new_block->size) ==
                               (uintptr_t)new_block->next) {
      new_block->size += new_block->next->size;
      new_block->next = new_block->next->next;
    }

    if (prev && ((uintptr_t)prev + prev->size) == (uintptr_t)new_block) {
      prev->size += new_block->size;
      prev->next = new_block->next;
    }
  }

  heap_size += new_region_size;
  return 1;
}

void heap_install(void) {
  kprintf("[heap] installing kernel heap...");
  if (heap_start != NULL) {
    panic("heap_install called multiple times");
  }

  heap_start = (void*)KERNEL_HEAP_START;
  heap_size = 0;

  if (!heap_expand_pages(KERNEL_INITIAL_PAGES)) {
    panic("heap: failed to allocate the initial kernel heap");
  }
  kprintf(" done!\n");
}

void* kmalloc(size_t sz) {
  if (sz == 0) return NULL;

  size_t payload = ALIGN_UP_HEAP(sz);
  size_t header_sz = ALIGN_UP_HEAP(sizeof(size_t));
  size_t total_sz = payload + header_sz;

  if (total_sz < MIN_ALLOC_SIZE) total_sz = MIN_ALLOC_SIZE;

  heap_block_t* prev = NULL;
  heap_block_t* curr = heap_head;

try_again:
  while (curr) {
    if (curr->size >= total_sz) {
      if (curr->size >= total_sz + MIN_ALLOC_SIZE) {
        heap_block_t* new_block = (heap_block_t*)((uintptr_t)curr + total_sz);
        new_block->size = curr->size - total_sz;
        new_block->next = curr->next;
        curr->size = total_sz;

        if (prev == NULL) {
          heap_head = new_block;
        } else {
          prev->next = new_block;
        }
      } else {
        if (prev == NULL) {
          heap_head = curr->next;
        } else {
          prev->next = curr->next;
        }
      }

      size_t* size_ptr = (size_t*)curr;
      *size_ptr = curr->size;

      void* user_ptr = (void*)((uintptr_t)curr + header_sz);
      return user_ptr;
    }

    prev = curr;
    curr = curr->next;
  }

  if (!heap_expand_pages(16)) {
    if (!heap_expand_pages(1)) {
      kprintf("kmalloc: Out of heap memory (requested %lx bytes)\n", sz);
      panic("out of memory");
    }
  }

  prev = NULL;
  curr = heap_head;
  goto try_again;
}

void* kmalloc_phys(size_t sz, uintptr_t* phys) {
  if (sz == 0) return NULL;
  void* virt_addr = kmalloc(sz);
  ASSERT(virt_addr != NULL);
  if (phys) {
    *phys = vmm_virt_to_phys(kernel_pagemap, (uintptr_t)virt_addr);
  }
  return virt_addr;
}

void kfree(void* ptr) {
  if (!ptr) return;

  size_t header_sz = ALIGN_UP_HEAP(sizeof(size_t));
  size_t* size_ptr = (size_t*)((uintptr_t)ptr - header_sz);
  void* block_start = (void*)size_ptr;
  size_t block_size = *size_ptr;

  if ((uintptr_t)block_start < (uintptr_t)heap_start ||
      (uintptr_t)block_start >= (uintptr_t)heap_start + heap_size) {
    panic("pointer out of bound!");
  }

  if (block_size < MIN_ALLOC_SIZE) {
    panic("invalid block size!");
  }

  if (((uintptr_t)block_start & (HEAP_ALIGNMENT - 1)) != 0) {
    panic("alignment error!");
  }

  heap_block_t* prev = NULL;
  heap_block_t* curr = heap_head;

  while (curr && (uintptr_t)curr < (uintptr_t)block_start) {
    prev = curr;
    curr = curr->next;
  }

  heap_block_t* freed = (heap_block_t*)block_start;
  freed->size = block_size;

  if (prev == NULL) {
    freed->next = heap_head;
    heap_head = freed;
  } else {
    freed->next = prev->next;
    prev->next = freed;
  }

  if (freed->next &&
      ((uintptr_t)freed + freed->size) == (uintptr_t)freed->next) {
    freed->size += freed->next->size;
    freed->next = freed->next->next;
  }

  if (prev && ((uintptr_t)prev + prev->size) == (uintptr_t)freed) {
    prev->size += freed->size;
    prev->next = freed->next;
  }
}