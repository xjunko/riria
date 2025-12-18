#include <riria/libk.h>

void* malloc(size_t size) {
  printk("[lib] malloc used, not implemented!\n");
  while (1) asm volatile("hlt");

  return 0;
}