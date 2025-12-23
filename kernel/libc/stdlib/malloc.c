#include <riria/libc.h>

void* malloc(size_t size) {
  kprintf("[lib] malloc used, not implemented!\n");
  while (1) asm volatile("hlt");

  return 0;
}