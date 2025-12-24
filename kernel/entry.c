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
#include <riria/drivers/pit.h>
#include <riria/drivers/ps2.h>
#include <riria/drivers/serial.h>
#include <riria/fs/vfs.h>
#include <riria/mem.h>
#include <riria/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  ps2_keyboard_install();
  framebuffer_install();
  ac97_install();

  // kmalloc test
  void* phys;
  void* virt_addr = kmalloc_phys(0x1000, (uintptr_t*)&phys);
  printf("kmalloc_phys: virt=%p phys=%p\n", virt_addr, (void*)phys);
  free(virt_addr);

  // test vfs
  // /dev/stdout
  vfs_file_t* serial_fs = vfs_open("/dev/stdout", 0, 0);
  if (!serial_fs) panic("failed to get /dev/stdout");
  int ret = vfs_write(serial_fs, "hello, from vfs!\n", 18);
  if (ret < 0) {
    printf("ret_code: %d\n", ret);
    panic("failed to write to /dev/stdout");
  }
  ret = vfs_close(serial_fs);
  if (ret < 0) {
    printf("ret_code: %d\n", ret);
    panic("failed to close /dev/stdout");
  }

  // /dev/zero
  void* buffer = malloc(16);
  memset(buffer, 0x69, 16);
  vfs_file_t* zero_fs = vfs_open("/dev/zero", 0, 0);
  if (!zero_fs) panic("failed to get /dev/zero");
  ret = vfs_read(zero_fs, buffer, 16);
  if (ret < 0) {
    printf("ret_code: %d\n", ret);
    panic("failed to read from /dev/zero");
  }
  ret = vfs_close(zero_fs);
  if (ret < 0) {
    printf("ret_code: %d\n", ret);
    panic("failed to close /dev/zero");
  }

  // check if the buffer is zeroed
  for (int i = 0; i < 16; i++) {
    if (((uint8_t*)buffer)[i] != 0) {
      panic("/dev/zero test failed!");
    }
  }

  // /init/human_48k_stereo.pcm
  vfs_file_t* tar_fs = vfs_open("/init/human_48k_stereo.pcm", 0, 0);
  if (!tar_fs) panic("failed to get /init/human_48k_stereo.pcm");
  buffer = malloc(4096);
  ret = vfs_read(tar_fs, buffer, 4096);
  if (ret < 0) {
    printf("ret_code: %d\n", (int)ret);
    panic("failed to read from /init/human_48k_stereo.pcm");
  }
  ret = vfs_close(tar_fs);
  if (ret < 0) {
    printf("ret_code: %d\n", (int)ret);
    panic("failed to close /init/human_48k_stereo.pcm");
  }

  for (size_t i = 0; i < 16; i++) {
    printf("%02x ", ((uint8_t*)buffer)[i]);
  }
  printf("\n");

  // test syscall
  asm volatile("int $33");  // ack 1
  asm volatile("int $0x80"
               :
               : "a"(4), "b"(1), "c"("hello, from syscall!\n"),
                 "d"(22));  // syscall write(4) to serial(1)

  printf("[sys] halted.\n");

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