#include <riria/cpu/gdt.h>
#include <riria/cpu/tss.h>
#include <riria/libk.h>
#include <stddef.h>
#include <stdint.h>

static struct {
  gdt_entry_t entries[6];
  gdt_ptr_t ptr;
  tss_entry_t tss;
} gdt __attribute__((used));

static tss_entry_t tss __attribute__((used, aligned(16)));

#define ENTRY(X) (gdt.entries[X])

void gdt_set_gate(uint8_t idx, uint64_t base, uint64_t limit, uint8_t access,
                  uint8_t granularity) {
  ENTRY(idx).base_low = (base & 0xFFFF);
  ENTRY(idx).base_middle = (base >> 16) & 0xFF;
  ENTRY(idx).base_high = (base >> 24) & 0xFF;

  ENTRY(idx).limit_low = (limit & 0xFFFF);
  ENTRY(idx).granularity = (limit >> 16) & 0x0F;

  ENTRY(idx).granularity |= (granularity & 0xF0);

  ENTRY(idx).access = access;
}

static void write_tss(uint32_t num, uint16_t ss0, uint32_t esp0);

void gdt_install(void) {
  gdt_ptr_t* gdtp = &gdt.ptr;
  gdtp->limit = sizeof(gdt.entries) - 1;
  gdtp->base = (uintptr_t)&ENTRY(0);

  gdt_set_gate(0, 0, 0, 0, 0);              // null
  gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xCF);  // code segment
  gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xCF);  // data segment
  gdt_set_gate(3, 0, 0xFFFFF, 0xFA, 0xCF);  // user code segment
  gdt_set_gate(4, 0, 0xFFFFF, 0xF2, 0xCF);  // user data segment

  write_tss(5, 0x10, 0x0);

  printk("[cpu] GDT=0x%x\n", gdtp->base);
  printk("[cpu] TSS=0x%x\n", (uintptr_t)&gdt.tss);

  // gdt_flush((uintptr_t)gdtp);
  // tss_flush();
}

static void write_tss(uint32_t num, uint16_t ss0, uint32_t esp0) {
  tss_entry_t* tss = &gdt.tss;
  uintptr_t base = (uintptr_t)tss;
  uintptr_t limit = base + sizeof(*tss);

  gdt_set_gate(num, base, limit, 0xE9, 0x00);
  memset(tss, 0x0, sizeof(*tss));

  tss->ss0 = ss0;
  tss->esp0 = esp0;
  tss->cs = 0x0b;
  tss->ss = 0x13;
  tss->ds = 0x13;
  tss->es = 0x13;
  tss->fs = 0x13;
  tss->gs = 0x13;

  tss->iomap_base = sizeof *tss;
}

void tss_set_stack(uintptr_t stack) {
  gdt.tss.esp0 = stack;
  printk("[cpu] TSS.ESP0=0x%x\n", stack);
}