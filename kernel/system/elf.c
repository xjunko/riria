#include <riria/elf.h>
#include <riria/mem.h>
#include <riria/process.h>
#include <riria/types.h>
#include <stdio.h>
#include <string.h>

uint64_t elf64_load(uint8_t* elf_data, size_t len) {
  if (len < sizeof(elf64_header_t)) {
    panic("elf64_load: invalid ELF header size\n");
    return 0;
  }

  elf64_header_t* header = (elf64_header_t*)elf_data;

  if (header->magic[0] != 0x7F || header->magic[1] != 'E' ||
      header->magic[2] != 'L' || header->magic[3] != 'F') {
    panic("invalid magic\n");
    return 0;
  }

  if (header->class_ != 2) {
    panic("not an elf64\n");
    return 0;
  }

  if (header->machine != 0x3E) {
    panic("not amd64\n");
    return 0;
  }

  uint64_t ph_offset = header->phoff;
  uint64_t ph_size = header->phentsize;
  uint16_t ph_count = header->phnum;

  for (size_t i = 0; i < ph_count; i++) {
    uint64_t ph_start = ph_offset + (i * ph_size);

    if (ph_start + ph_size > len) {
      panic("header out of bound!\n");
      return 0;
    }

    elf64_phdr_t* phdr = (elf64_phdr_t*)(elf_data + ph_start);

    if (phdr->type != PT_LOAD) {
      continue;
    }

    uint64_t flags = PTE_PRESENT | PTE_USER;

    if (phdr->flags & PF_W) {
      flags |= PTE_WRITABLE;
    }

    if (!(phdr->flags & PF_X)) {
      printf("[elf] setting NX for segment %d\n", i);
      flags |= PTE_NX;
    }

    uint64_t start_page = phdr->vaddr & ~(PAGE_SIZE - 1);
    uint64_t end_addr = (phdr->vaddr + phdr->memsz);
    uint64_t end_page = (end_addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    ASSERT(start_page < end_page);
    ASSERT(end_page > start_page);
    ASSERT(start_page % PAGE_SIZE == 0);
    ASSERT(end_page % PAGE_SIZE == 0);
    ASSERT(start_page != 0);

    uintptr_t addr = start_page;
    while (addr < end_page) {
      uintptr_t page = (uintptr_t)pmm_allocate();
      vmm_map_page(process_get_current()->pagemap, addr, page, flags);
      addr += PAGE_SIZE;
    }

    uint64_t file_start = phdr->offset;
    uint64_t file_end = file_start + phdr->filesz;

    if (file_end > len) {
      panic("segment out of bound!\n");
      return 0;
    }

    ASSERT(elf_data + file_start != NULL);
    ASSERT((void*)phdr->vaddr != NULL);
    ASSERT(phdr->filesz > 0);
    memcpy((void*)phdr->vaddr, elf_data + file_start, phdr->filesz);

    if (phdr->memsz > phdr->filesz) {
      memset((void*)(phdr->vaddr + phdr->filesz), 0,
             phdr->memsz - phdr->filesz);
    }
  }
  return header->entry_point;
}