#pragma once
#include <riria/types.h>

typedef struct gdt_entry {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t granularity;  // low 4 bits: limit_high, high 4 bits: flags
  uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct gdt_ptr {
  uint16_t limit;
  uintptr_t base;
} __attribute__((packed)) gdt_ptr_t;

void gdt_install(void);
void gdt_set_gate(uint8_t, uint64_t, uint64_t, uint8_t, uint8_t);

extern void gdt_flush(uintptr_t);
