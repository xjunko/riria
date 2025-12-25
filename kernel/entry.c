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

extern void switch_to_user(void);

void _userspace_thread(void) {
  uintptr_t start_virt = 0x10000;
  size_t page_size = PAGE_SIZE;
  size_t needed_mem = 1 * 1024 * 1024;

  for (size_t i = start_virt; i < start_virt + needed_mem; i += PAGE_SIZE) {
    uintptr_t page = (uintptr_t)pmm_allocate();
    vmm_map_page(process_get_current()->pagemap, i, page,
                 PTE_PRESENT | PTE_USER | PTE_WRITABLE);
  }

  uintptr_t stack_top = 0x80000000;
  size_t stack_size = PAGE_SIZE;

  for (uintptr_t i = stack_top - stack_size; i < stack_top; i += PAGE_SIZE) {
    uintptr_t pa = (uintptr_t)pmm_allocate();
    vmm_map_page(process_get_current()->pagemap, i, pa,
                 PTE_PRESENT | PTE_USER | PTE_WRITABLE);
  }

  vfs_file_t* elf_file = vfs_open("/init/test.bin", 0, 0);
  ASSERT(elf_file);
  size_t elf_size = vfs_read(elf_file, (void*)0x10000, needed_mem);

  asm volatile("swapgs");
  switch_to_user();
}

void _thread_test1(void) {
  // test audio driver
  ac97_set_volume(50);
  vfs_file_t* audio_fs = vfs_open("/init/human_48k_stereo.pcm", 0, 0);
  ASSERT(audio_fs);

  vfs_file_t* ac97_fs = vfs_open("/dev/audio", 0, 0);
  ASSERT(ac97_fs);

  size_t chunk_size = 128000;
  void* buffer = malloc(chunk_size);
  int pcm_pos = 0;

  for (;;) {
    while (!ac97_can_write())
      ;
    vfs_seek(audio_fs, pcm_pos, 0);
    int bytes_read = vfs_read(audio_fs, buffer, chunk_size);
    ASSERT(bytes_read >= 0);

    if (bytes_read == 0) break;

    int total_written = 0;
    while (total_written < bytes_read) {
      int written = vfs_write(ac97_fs, (uint8_t*)buffer + total_written,
                              bytes_read - total_written);

      total_written += written;
    }
    pcm_pos += bytes_read;
  }
  int ret = vfs_close(audio_fs);
  ASSERT(ret >= 0);
}

void _thread_test2(void) {
  while (1) {
    printf("hello from thread %d\n", process_get_current()->id);
  }
}

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

  // syscall
  syscall_install();

  // kmalloc test
  void* phys;
  void* virt_addr = kmalloc_phys(0x1000, (uintptr_t*)&phys);
  printf("kmalloc_phys: virt=%p phys=%p\n", virt_addr, (void*)phys);
  free(virt_addr);

  // test vfs
  // /dev/stdout
  vfs_file_t* serial_fs = vfs_open("/dev/stdout", 0, 0);
  ASSERT(serial_fs);
  int ret = vfs_write(serial_fs, "hello, from vfs!\n", 18);
  ASSERT(ret >= 0);
  ret = vfs_close(serial_fs);
  ASSERT(ret >= 0);

  // /dev/zero
  void* buffer = malloc(16);
  memset(buffer, 0x69, 16);
  vfs_file_t* zero_fs = vfs_open("/dev/zero", 0, 0);
  ASSERT(zero_fs);
  ret = vfs_read(zero_fs, buffer, 16);
  ASSERT(ret >= 0);
  ret = vfs_close(zero_fs);
  ASSERT(ret >= 0);

  // check if the buffer is zeroed
  for (int i = 0; i < 16; i++) {
    if (((uint8_t*)buffer)[i] != 0) {
      panic("/dev/zero test failed!");
    }
  }

  // /init/human_48k_stereo.pcm
  vfs_file_t* tar_fs = vfs_open("/init/human_48k_stereo.pcm", 0, 0);
  ASSERT(tar_fs);
  buffer = malloc(4096);
  ret = vfs_read(tar_fs, buffer, 4096);
  ASSERT(ret >= 0);
  ret = vfs_close(tar_fs);
  ASSERT(ret >= 0);

  for (size_t i = 0; i < 16; i++) {
    printf("%02x ", ((uint8_t*)buffer)[i]);
  }
  printf("\n");

  // test syscall
  asm volatile("int $33");  // ack 1

  // thread testing
  process_create(NULL, NULL);
  process_create(_thread_test1, NULL);
  process_create(_thread_test2, NULL);

  // user process
  pagemap_t* user_pagemap = vmm_new_pagemap();
  process_create(_userspace_thread, user_pagemap);

  printf("[sys] halted.\n");
  uint32_t i = 0;
  while (1) {
    i++;

    // i call this, "minecraft"
    for (uint32_t x = (i % 640) + 100; x < (i % 640) + 200; x++) {
      for (uint32_t y = (i % 400) + 100; y < (i % 400) + 200; y++) {
        framebuffer_draw_pixel(x, y, (i << 16) | (i << 8) | i);
      }
    }
  }
}