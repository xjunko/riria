#include <riria/libc.h>
#include <riria/mem.h>

void* malloc(size_t size) { return kmalloc(size); }
