#include <riria/elf.h>
#include <riria/mem.h>
#include <riria/process.h>
#include <riria/types.h>
#include <stdio.h>
#include <string.h>

elf_info_t elf64_load(pagemap_t* pagemap, uint8_t* elf_data, size_t len) {
  if (len < sizeof(elf64_header_t)) {
    panic("elf64_load: invalid ELF header size\n");
  }

  elf64_header_t* header = (elf64_header_t*)elf_data;

  if (header->magic[0] != 0x7F || header->magic[1] != 'E' ||
      header->magic[2] != 'L' || header->magic[3] != 'F') {
    panic("invalid magic\n");
  }

  if (header->class_ != 2) {
    panic("not an elf64\n");
  }

  if (header->machine != 0x3E) {
    panic("not amd64\n");
  }

  uint64_t ph_offset = header->phoff;
  uint64_t ph_size = header->phentsize;
  uint16_t ph_count = header->phnum;

  uint64_t base_addr = UINT64_MAX;
  uint64_t end_addr = 0x0;

  for (size_t i = 0; i < ph_count; i++) {
    uint64_t ph_start = ph_offset + (i * ph_size);

    if (ph_start + ph_size > len) {
      panic("header out of bound!\n");
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
      flags |= PTE_NX;
    }

    uint64_t start_page = phdr->vaddr & ~(PAGE_SIZE - 1);
    uint64_t segment_end = phdr->vaddr + phdr->memsz;
    uint64_t end_page = (segment_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    ASSERT(start_page < end_page);
    ASSERT(end_page > start_page);
    ASSERT(start_page % PAGE_SIZE == 0);
    ASSERT(end_page % PAGE_SIZE == 0);
    ASSERT(start_page != 0);

    uintptr_t addr = start_page;
    while (addr < end_page) {
      uintptr_t page = (uintptr_t)pmm_allocate();
      ASSERT(page);
      vmm_map_page(pagemap, addr, page, flags);
      addr += PAGE_SIZE;
    }

    uint64_t file_start = phdr->offset;
    uint64_t file_end = file_start + phdr->filesz;

    if (file_end > len) {
      panic("segment out of bound!\n");
    }

    ASSERT(elf_data + file_start != NULL);
    ASSERT((void*)phdr->vaddr != NULL);
    ASSERT(phdr->filesz > 0);
    vmm_map_copy(pagemap, phdr->vaddr, elf_data + file_start, phdr->filesz);

    if (phdr->memsz > phdr->filesz) {
      vmm_map_zero(pagemap, phdr->vaddr + phdr->filesz,
                   phdr->memsz - phdr->filesz);
    }

    if (phdr->vaddr < base_addr) base_addr = phdr->vaddr;
    if (segment_end > end_addr) end_addr = segment_end;
  }

  elf_info_t info;
  info.entry = header->entry_point;
  info.size = end_addr - base_addr;
  info.base = base_addr;
  printf("[elf] loaded elf64: entry=0x%lx size=0x%lx\n", info.entry, info.size);

  return info;
}