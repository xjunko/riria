#include <riria/cpu/gdt.h>
#include <riria/cpu/tss.h>
#include <riria/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__attribute__((aligned(16))) uint8_t kernel_stack[16384];
__attribute__((aligned(16))) uint8_t user_stack[65536];
__attribute__((aligned(16))) uint8_t df_stack[65536];
__attribute__((aligned(16))) uint8_t nmi_stack[65536];

static struct {
  gdt_entry_t entries[7];
  gdt_ptr_t ptr;
  tss_entry_t tss;
} gdt __attribute__((used));

static tss_entry_t tss __attribute__((used, aligned(16)));

#define ENTRY(X) (gdt.entries[X])

void gdt_set_gate(uint8_t idx, uint64_t base, uint64_t limit, uint8_t access,
                  uint8_t granularity) {
  // in 64bit, base and limit value are ignored.
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

  gdt_set_gate(0, 0, 0, 0, 0);        // 0x0 - null
  gdt_set_gate(1, 0, 0, 0x9A, 0x20);  // 0x8 - code segment
  gdt_set_gate(2, 0, 0, 0x92, 0x0);   // 0x10 - data segment
  gdt_set_gate(3, 0, 0, 0xF2, 0x20);  // 0x18 - user code segment
  gdt_set_gate(4, 0, 0, 0xFA, 0x0);   // 0x20 - user data segment
  write_tss(5, 0x10, 0x0);            // 0x28 - TSS segment (1)

  printf("[cpu] GDT=0x%x\n", gdtp->base);
  printf("[cpu] TSS=0x%x\n", (uintptr_t)&gdt.tss);
  printf("[cpu] GDT INIT...");

  gdt_flush((uintptr_t)gdtp);
  tss_flush();

  printf(" OK!\n");  // it's not a proper osdev project if it doesnt have GDT
                     // INIT... OK /j
}

static void write_tss(uint32_t num, uint16_t ss0, uint32_t esp0) {
  UNUSED(ss0);
  UNUSED(esp0);

  tss_entry_t* tss = &gdt.tss;
  uintptr_t base = (uintptr_t)tss;
  uintptr_t limit = sizeof(*tss) - 1;

  gdt_set_gate(num, base, limit, 0x89, 0x00);
  memset(tss, 0x0, sizeof(*tss));

  tss->ist1 = (uint64_t)(df_stack + sizeof(df_stack));
  tss->ist2 = (uint64_t)(nmi_stack + sizeof(nmi_stack));
  tss->rsp0 = (uint64_t)(kernel_stack + sizeof(kernel_stack));

  // HACK: bit cursed but it does the job for now
  uint32_t* hi = (uint32_t*)&gdt.entries[num + 1];
  hi[0] = (base >> 32);
  hi[1] = 0;
}

void tss_set_stack(uintptr_t stack) {
  gdt.tss.rsp0 = stack;
  printf("[cpu] TSS.ESP0=0x%x\n", stack);
}