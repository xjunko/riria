#include <riria/cpu/irq.h>
#include <riria/types.h>
#include <stdio.h>

__attribute__((noreturn)) void panic(const char* message) {
  int_disable();
  kprintf("[err] KERNEL PANIC: %s\n", message);
  while (1) asm volatile("hlt");
}