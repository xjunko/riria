#pragma once
#include <riria/types.h>

typedef struct idt_entry {
  uint16_t base_low;
  uint16_t segment_selector;
  uint8_t reserved;
  uint8_t flags;
  uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

typedef struct idt_ptr {
  uint16_t limit;
  uintptr_t base;
} __attribute__((packed)) idt_ptr_t;

typedef void (*idt_gate)(void);

void idt_install(void);
void idt_set_gate(uint8_t, idt_gate, uint16_t, uint8_t);
extern void idt_flush(uintptr_t);
