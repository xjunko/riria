#include <riria/libc.h>
#include <riria/mem.h>

void free(void* ptr) { kfree(ptr); }