#include <riria/cpu/idt.h>
#include <riria/cpu/isr.h>
#include <riria/cpu/regs.h>
#include <riria/syscall.h>
#include <stdio.h>

#define ISR_COUNT 32

static struct {
  size_t idx;
  void (*stub)(void);
} isrs[32 + 1] __attribute__((used));

static isr_callback isr_callbacks[256] = {0};

void isr_install_handler(size_t isr, isr_callback callback) {
  printf("[isr] installing handler for ISR %d\n", isr);
  isr_callbacks[isr] = callback;
}

void isr_uninstall_handler(size_t isr) { isr_callbacks[isr] = 0; }

static void _gpf_fault_handler(regs_t* r) {
  printf("[isr] general protection fault at 0x%lx\n", r->rip);
  print_regs(r);
  panic("general protection fault");
}

void isr_install(void) {
  // should work now
  ISR_SET(0);
  ISR_SET(1);
  ISR_SET(2);
  ISR_SET(3);
  ISR_SET(4);
  ISR_SET(5);
  ISR_SET(6);
  ISR_SET(7);
  ISR_SET(8);
  ISR_SET(9);
  ISR_SET(10);
  ISR_SET(11);
  ISR_SET(12);
  ISR_SET(13);
  ISR_SET(14);
  ISR_SET(15);
  ISR_SET(16);
  ISR_SET(17);
  ISR_SET(18);
  ISR_SET(19);
  ISR_SET(20);
  ISR_SET(21);
  ISR_SET(22);
  ISR_SET(23);
  ISR_SET(24);
  ISR_SET(25);
  ISR_SET(26);
  ISR_SET(27);
  ISR_SET(28);
  ISR_SET(29);
  ISR_SET(30);
  ISR_SET(31);

  idt_set_gate(13, _isr13, 0x08, 0x8E);  // general protection fault
  isr_install_handler(13, _gpf_fault_handler);

  idt_set_gate(14, _isr14, 0x08, 0x8E);  // page fault

  for (int i = 0; i <= ISR_COUNT; i++) {
    idt_set_gate(isrs[i].idx, isrs[i].stub, 0x08, 0x8E);
  }
}

void isr_handler(regs_t* r) {
  if (r->cs & 0x3) {
    asm volatile("swapgs");
  }

  printf("[isr] interrupt received: 0x%x (%d)\n", r->int_no, r->int_no);
#ifdef DEBUG
  print_regs(r);
  printf("[isr] rip: 0x%lx cs: 0x%lx rsp: 0x%lx ss: 0x%lx\n", r->rip, r->cs,
         r->rsp, r->ss);
#endif

  isr_callback handler = isr_callbacks[r->int_no];
  if (handler) {
    handler(r);
  } else {
    printf("[isr] unhandled interrupt: %d\n", r->int_no);
  }

  if (r->cs & 0x3) {
    asm volatile("swapgs");
  }
}