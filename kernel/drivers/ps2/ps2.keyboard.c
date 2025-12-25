#include <riria/cpu/io.h>
#include <riria/cpu/irq.h>
#include <riria/drivers/ps2.h>
#include <stdio.h>

#define KB_IRQ 0x1
#define KB_DEVICE 0x60
#define KB_PENDING 0x64

volatile char input_buffer[KB_BUFFER_SIZE];
volatile size_t input_buffer_index = 0;
volatile int shift_pressed = 0;

static void keyboard_wait(void) {
  while (inb(KB_PENDING) & 0x2)
    ;
}

static int keyboard_handler(struct regs* r) {
  UNUSED(r);

  unsigned char scancode = 0;
  if (inb(KB_PENDING) & 0x1) {
    scancode = inb(KB_DEVICE);
  }

  if (scancode == 0x2A || scancode == 0x36) {
    shift_pressed = 1;
  } else if (scancode == 0xAA || scancode == 0xB6) {
    shift_pressed = 0;
  }

  if (scancode && shift_pressed) {
    KB_PUSH(ascii_set_shifted[scancode]);
  } else if (scancode) {
    KB_PUSH(ascii_set_normal[scancode]);
  }

  irq_ack(KB_IRQ);

  return 1;
}

int ps2_keyboard_install(void) {
  irq_install_handler(KB_IRQ, keyboard_handler, "ps2.keyboard");
  return 0;
}

int ps2_keyboard_uninstall(void) { return 0; }
