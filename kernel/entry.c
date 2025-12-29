#include <riria/boot.h>
#include <riria/cpu/cpuid.h>
#include <riria/cpu/features.h>
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

extern void userspace_shell(void);

bool cpuid_has_sse(void) {
  cpuid_info_t info = cpuid(1, 0);
  return (info.edx & (1 << 25)) != 0;
}

bool cpuid_has_fsgsbase(void) {
  cpuid_info_t info = cpuid(7, 0);
  return (info.ebx & (1 << 0)) != 0;
}

void check_cpu_features(void) {
  if (!cpuid_has_fsgsbase()) {
    panic("cpu does not support FSGSBASE instructions!");
  } else {
    write_cr4(read_cr4() | (1 << 16));  // FSGSBASE
    printf("[cpu] FSGSBASE instructions enabled\n");
  }

  if (!cpuid_has_sse()) {
    panic("cpu does not support SSE!");
  } else {
    uint64_t cr0 = read_cr0();
    cr0 &= ~(1ULL << 2);  // clear EM to allow FPU instructions
    cr0 |= (1ULL << 1);   // set MP so WAIT/FWAIT checks TS bit
    write_cr0(cr0);

    uint64_t cr4 = read_cr4();
    cr4 |= (1ULL << 9);   // OSFXSR enables SSE instructions
    cr4 |= (1ULL << 10);  // OSXMMEXCPT enables SSE exceptions
    write_cr4(cr4);
    printf("[cpu] SSE enabled\n");
  }
}

void kmain(void) {
  serial_install();
  check_cpu_features();
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
  process_create(userspace_shell, NULL);

  HALT();
}