#include <riria/boot.h>
#include <riria/cpu/gdt.h>
#include <riria/cpu/idt.h>
#include <riria/cpu/io.h>
#include <riria/cpu/irq.h>
#include <riria/cpu/isr.h>
#include <riria/cpu/tss.h>
#include <riria/drivers/ac97.h>
#include <riria/drivers/framebuffer.h>
#include <riria/drivers/pci.h>
#include <riria/drivers/ps2.h>
#include <riria/drivers/serial.h>
#include <riria/fs/vfs.h>
#include <riria/mem.h>
#include <riria/process.h>
#include <riria/syscall.h>
#include <riria/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void kernel_shell(void);

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

  // vfs
  vfs_install();

  // basic drivers
  framebuffer_install();
  ps2_keyboard_install();
  ac97_install();

  // syscall
  syscall_install();

  // idle process
  process_create(NULL, NULL);

  // shell
  process_create(kernel_shell, NULL);

  for (;;)
    ;
}