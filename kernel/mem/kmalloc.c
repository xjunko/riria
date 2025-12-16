#include <riria/kmalloc.h>
#include <stddef.h>
#include <stdint.h>

extern void* end;
uintptr_t bump_ptr = (uintptr_t)&end;

void kmalloc_start_at(uintptr_t addr) { bump_ptr = addr; }

void* kmalloc(size_t sz) {
  uintptr_t addr = bump_ptr;
  bump_ptr += sz;
  return (void*)addr;
}