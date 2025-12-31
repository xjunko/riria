#include <riria/cpu/irq.h>
#include <riria/types.h>
#include <stdio.h>

__attribute__((noreturn)) void panic(const char* message) {
  IRQ_OFF;
  printf(ERROR "[ error] KERNEL PANIC: %s\n", message);
  while (1) asm volatile("hlt");
}