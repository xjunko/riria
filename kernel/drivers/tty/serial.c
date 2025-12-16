#include <riria/cpu/io.h>
#include <riria/serial.h>

#define COM1 0x3F8

void serial_install(void) {
  outb(COM1 + 1, 0x00);  // dissable interrupts
  outb(COM1 + 3, 0x80);  // enable dlab
  outb(COM1 + 0, 0x03);  // set baud rate lb
  outb(COM1 + 1, 0x00);  // set baud rate hb
  outb(COM1 + 3, 0x03);  // 8 bits, no parity, one stop bit
  outb(COM1 + 2, 0xC7);  // enable fifo
  outb(COM1 + 4, 0x0B);  // irqs enabled
}

static void serial_wait(void) {
  while (!(inb(COM1 + 5) & 0x20))
    ;
}

void serial_write(char c) {
  serial_wait();
  outb(COM1, c);
}