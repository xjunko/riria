#include <riria/mem.h>
#include <stdio.h>

void free(void* ptr) { kfree(ptr); }