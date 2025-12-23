#include <riria/cpu/idt.h>
#include <riria/cpu/isr.h>
#include <riria/libc.h>

static struct {
  idt_entry_t entries[256];
  idt_ptr_t ptr;
} idt __attribute__((used));

#define ENTRY(X) (idt.entries[X])

void idt_set_gate(uint8_t num, idt_gate base, uint16_t segment_selector,
                  uint8_t flags) {
  ENTRY(num).segment_selector = segment_selector;

  uintptr_t addr = (uintptr_t)base;
  ENTRY(num).base_low = (uint16_t)(addr & 0xFFFF);
  ENTRY(num).base_middle = (uint16_t)((addr >> 16) & 0xFFFF);
  ENTRY(num).base_high = (uint32_t)((addr >> 32) & 0xFFFFFFFF);

  ENTRY(num).ist = 0;
  ENTRY(num).flags = flags;
  ENTRY(num).reserved = 0;
}

void idt_install(void) {
  idt_ptr_t* idtp = &idt.ptr;
  idtp->limit = sizeof(idt.entries) - 1;
  idtp->base = (uintptr_t)&ENTRY(0);
  memset(&ENTRY(0), 0, sizeof(idt.entries));

  printk("[cpu] IDT=0x%x\n", idtp->base);
  printk("[cpu] IDT INIT...");
  idt_flush((uintptr_t)idtp);
  printk(" OK!\n");
}