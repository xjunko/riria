#include <riria/cpu/io.h>
#include <riria/cpu/irq.h>
#include <riria/drivers/ps2.h>
#include <riria/libc.h>

#define KB_IRQ 0x1
#define KB_DEVICE 0x60
#define KB_PENDING 0x64

static void keyboard_wait(void) {
  while (inb(KB_PENDING) & 0x2)
    ;
}

static int keyboard_handler(struct regs* r) {
  printk("[ps2] keyboard handler called \n");

  unsigned char scancode;
  if (inb(KB_PENDING) & 0x1) {
    scancode = inb(KB_DEVICE);
  }

  printk("[ps2] scancode: 0x%x\n", scancode);

  irq_ack(KB_IRQ);

  return 1;
}

int ps2_keyboard_install(void) {
  irq_install_handler(KB_IRQ, keyboard_handler, "ps2.keyboard");
  return 0;
}

int ps2_keyboard_uninstall(void) { return 0; }
