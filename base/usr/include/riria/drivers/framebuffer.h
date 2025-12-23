#pragma once
#include <stdint.h>

void framebuffer_install(void);
void framebuffer_draw_pixel(int, int, uint32_t);

// flanterm wrapper
void framebuffer_write(const char[]);