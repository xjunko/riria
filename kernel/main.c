#include <riria/boot.h>
#include <riria/cpu/gdt.h>
#include <riria/cpu/idt.h>
#include <riria/cpu/irq.h>
#include <riria/cpu/isr.h>
#include <riria/cpu/tss.h>
#include <riria/elf.h>
#include <riria/framebuffer.h>
#include <riria/libk.h>
#include <riria/mem.h>
#include <riria/ps2.h>
#include <riria/serial.h>

#include "misc/elf_test.c"

extern void* end;

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

  uintptr_t last_end = (uintptr_t)&end;
  if ((uintptr_t)mb_info > last_end) {
    uint32_t mb2_size = *(uint32_t*)mb_info;

    printk("[mem] mb_info is bigger than kernel's end\n");
    printk("[mem] mb_info=0x%x (sz=0x%x) > cur_end=0x%x \n", (uintptr_t)mb_info,
           mb2_size, last_end);

    last_end = (uintptr_t)mb_info + mb2_size;
    printk("[mem] last_end=0x%x \n", last_end);
  }

  // NOTE: this will probably collide with tss_set_stack
  kmalloc_start_at(last_end);

  // paging
  paging_initialize();
  paging_finalize();

  // TODO: hardcoded, refer to how toaruos does it.
  uint32_t kernel_stack =
      ((uint32_t)&last_end + 0x4000) & ~0xF;  // 16KB after kernel end, aligned
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

  // try accessing invalid memory
  uint32_t* invalid_ptr = (uint32_t*)0xDEADBEEF;
  // should page_fault here.
  uint32_t val = *invalid_ptr;
  printk("invalid read returned 0x%x\n", val);

  while (1) {
  }
}