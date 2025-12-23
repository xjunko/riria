#include <riria/boot.h>
#include <riria/cpu/gdt.h>
#include <riria/cpu/idt.h>
#include <riria/cpu/irq.h>
#include <riria/cpu/isr.h>
#include <riria/cpu/tss.h>
#include <riria/drivers/framebuffer.h>
#include <riria/drivers/ps2.h>
#include <riria/drivers/serial.h>
#include <riria/libc.h>
#include <riria/mem.h>
#include <riria/types.h>

void kmain(void) {
  serial_install();  // will be used for printk
  boot_verify();

  // bare minimum
  gdt_install();
  idt_install();
  irq_install();
  isr_install();

  // memory
  pmm_install();
  vmm_install();
  heap_install();

  // basic drivers
  ps2_keyboard_install();
  framebuffer_install();

  asm volatile("int $33");  // ack 1
  asm volatile("int $0x80"
               :
               : "a"(4), "b"(1), "c"("Hello, Riria!\n"),
                 "d"(15));  // syscall write(4) to serial(1)

  kprintf("[sys] halted.\n");

  int i = 0;
  while (1) {
    i++;

    // i call this, "minecraft"
    for (int x = (i % 640) + 100; x < (i % 640) + 200; x++) {
      for (int y = (i % 400) + 100; y < (i % 400) + 200; y++) {
        framebuffer_draw_pixel(x, y, (i << 16) | (i << 8) | i);
      }
    }
  }
}