#include <riria/cpu/idt.h>
#include <riria/cpu/isr.h>
#include <riria/libk.h>

static struct {
  idt_entry_t entries[256];
  idt_ptr_t ptr;
} idt __attribute__((used));

#define ENTRY(X) (idt.entries[X])

void idt_set_gate(uint8_t num, idt_gate base, uint16_t segment_selector,
                  uint8_t flags) {
  ENTRY(num).base_low = (uintptr_t)base & 0xFFFF;
  ENTRY(num).base_high = ((uintptr_t)base >> 16) & 0xFFFF;
  ENTRY(num).segment_selector = segment_selector;
  ENTRY(num).reserved = 0;

  // Use the flags as-is (DPL is encoded in flags parameter)
  ENTRY(num).flags = flags;
}

void idt_install(void) {
  idt_ptr_t* idtp = &idt.ptr;
  idtp->limit = sizeof(idt.entries) - 1;
  idtp->base = (uintptr_t)&ENTRY(0);
  memset(&ENTRY(0), 0, sizeof(idt.entries));

  printk("[cpu] IDT=0x%x\n", idtp->base);

  // idt_flush((uintptr_t)idtp);
}