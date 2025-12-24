#pragma once
#include <riria/types.h>

typedef struct tss_entry {
  uint32_t reserved0;
  uint64_t rsp0;
  uint64_t rsp1;
  uint64_t rsp2;
  uint64_t reserved1;
  uint64_t ist[7];
  uint64_t reserved2;
  uint16_t reserved3;
  uint16_t io_map_base;
} __attribute__((packed)) tss_entry_t;

void tss_set_stack(uintptr_t esp0);
void write_tss(uint32_t num);

extern void tss_flush(void);
