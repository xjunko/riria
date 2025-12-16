#pragma once
#include <riria/thirdparty/multiboot2.h>
#include <stddef.h>
#include <stdint.h>

typedef void (*multiboot_callback)(void *);

void mb2_parse(uint32_t, uint32_t, multiboot_callback);
