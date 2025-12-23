#include <riria/libc.h>

void* malloc(size_t size) {
  kprintf("[lib] malloc used, not implemented!\n");
  panic("malloc not implemented");

  return 0;
}