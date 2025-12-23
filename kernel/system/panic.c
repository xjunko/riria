#include <riria/cpu/irq.h>
#include <riria/libc.h>
#include <riria/types.h>

void panic(const char* message) {
  int_disable();
  kprintf("[err] KERNEL PANIC: %s\n", message);
  while (1) asm volatile("hlt");
}