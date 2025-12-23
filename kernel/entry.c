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

  // basic drivers
  ps2_keyboard_install();
  framebuffer_install();

  asm volatile("int $33");  // ack 1
  asm volatile("int $0x80"
               :
               : "a"(4), "b"(1), "c"("Hello, Riria!\n"),
                 "d"(15));  // syscall write(4) to serial(1)

  kprintf("[sys] halted.\n");
  while (1) {
  }
}