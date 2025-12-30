#pragma once
#include <stdint.h>

typedef struct {
  void* addr;
  void* buffer;
  uint32_t width;
  uint32_t height;
  uint32_t bpp;
  uint32_t pitch;

  struct {
    uint8_t red_size, red_shift;
    uint8_t green_size, green_shift;
    uint8_t blue_size, blue_shift;
  } masks;

  struct flanterm_context* ft_ctx;
} fb_info_t;

extern fb_info_t fb_info;

void framebuffer_install(void);
void framebuffer_draw_pixel(int, int, uint32_t);

// flanterm wrapper
void framebuffer_write(const char[]);