#include <riria/boot.h>
#include <riria/cpu/gdt.h>
#include <riria/cpu/idt.h>
#include <riria/cpu/irq.h>
#include <riria/cpu/isr.h>
#include <riria/cpu/tss.h>
#include <riria/elf.h>
#include <riria/framebuffer.h>
#include <riria/libk.h>
#include <riria/ps2.h>
#include <riria/serial.h>

#include "misc/elf_test.c"

int kmain(uint32_t mb_magic, uint32_t mb_info) {
  serial_install();  // will be used for printk

  // check if stuff went wrong
  if (mb_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    printk("[boot] invalid bootloader header, halting. \n");
    while (1) asm volatile("hlt");
  }

  // bare minimum
  gdt_install();
  idt_install();
  irq_install();
  isr_install();

  // TODO: hardcoded, refer to how toaruos does it.
  // Set up kernel stack for returning from user mode
  extern uint32_t end;  // defined in linker script
  uint32_t kernel_stack =
      ((uint32_t)&end + 0x4000) & ~0xF;  // 16KB after kernel end, aligned
  tss_set_stack(kernel_stack);

  // drivers
  framebuffer_install(mb_info);
  ps2_keyboard_install();

  // safe now
  printk("hello world!\n");
  printk("six seven = %d\n", 67);
  printk("0xdead = 0x%x\n", 0xDEAD);
  printk("string = %s\n", "hell yeah brother");

  // try loading the elf
  // printk("[elf] loading test ELF...\n");
  // printk("[elf] length is %d bytes\n", sizeof(test_elf));
  // load_elf(test_elf);
  // printk("[elf] returned to kernel!\n");

  // try calling 0x80
  // asm volatile("int $0x80");

  while (1) {
  }
}