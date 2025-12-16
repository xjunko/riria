#include <riria/cpu/idt.h>
#include <riria/cpu/isr.h>
#include <riria/cpu/regs.h>
#include <riria/libk.h>
#include <riria/syscall.h>

#define ISR_COUNT 32

static struct {
  size_t idx;
  void (*stub)(void);
} isrs[32 + 1] __attribute__((used));

static isr_callback isr_callbacks[256] = {0};

void isr_install_handler(size_t isr, isr_callback callback) {
  printk("[isr] installing handler for ISR %d\n", isr);
  isr_callbacks[isr] = callback;
}

void isr_uninstall_handler(size_t isr) { isr_callbacks[isr] = 0; }

void isr_install(void) {
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
  ISR_SET(128);

  // syscall handler
  idt_set_gate(0x80, _isr128, 0x08, 0xEE);
  isr_install_handler(0x80, syscall_handler);

  //   for (int i = 0; i <= ISR_COUNT; i++) {
  //     idt_set_gate(isrs[i].idx, isrs[i].stub, 0x08, 0x8E);
  //   }
}

void isr_handler(regs_t* r) {
  printk("[isr] interrupt received: 0x%x (%d)\n", r->int_no, r->int_no);

  isr_callback handler = isr_callbacks[r->int_no];
  if (handler) {
    handler(r);
  } else {
    printk("[isr] unhandled interrupt: %d\n", r->int_no);
  }
}