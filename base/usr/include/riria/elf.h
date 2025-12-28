#pragma once
#include <riria/process.h>
#include <riria/types.h>

typedef struct elf64_header {
  uint8_t magic[4];
  uint8_t class_;
  uint8_t endian;
  uint8_t version;
  uint8_t os_abi;
  uint8_t pad[8];
  uint16_t elf_type;
  uint16_t machine;
  uint32_t elf_version;
  uint64_t entry_point;
  uint64_t phoff;
  uint64_t shoff;
  uint32_t flags;
  uint16_t ehsize;
  uint16_t phentsize;
  uint16_t phnum;
  uint16_t shentsize;
  uint16_t shnum;
  uint16_t shstrndx;
} elf64_header_t;

typedef struct elf64_phdr {
  uint32_t type;
  uint32_t flags;
  uint64_t offset;
  uint64_t vaddr;
  uint64_t paddr;
  uint64_t filesz;
  uint64_t memsz;
  uint64_t align;
} elf64_phdr_t;

#define PT_LOAD 1
#define PF_X 1
#define PF_W 2
#define PF_R 4

uint64_t elf64_load(pagemap_t*, uint8_t*, size_t);