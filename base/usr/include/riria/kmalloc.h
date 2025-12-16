#pragma once
#include <stddef.h>
#include <stdint.h>

void kmalloc_start_at(uintptr_t);
void* kmalloc(size_t);