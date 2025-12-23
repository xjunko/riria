#include <riria/boot.h>
#include <riria/drivers/framebuffer.h>
#include <riria/libc.h>
#include <riria/types.h>

#include "flanterm/flanterm.h"
#include "flanterm/flanterm_backends/fb.h"

static struct {
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
} fb_info __attribute__((used));

void framebuffer_install(void) {
  if (framebuffer_request.response == NULL ||
      framebuffer_request.response->framebuffer_count < 1) {
    panic("no framebuffer found!");
  }

  struct limine_framebuffer* fb = framebuffer_request.response->framebuffers[0];
  fb_info.addr = fb->address;
  fb_info.buffer = fb->address;

  fb_info.width = (uint32_t)fb->width;
  fb_info.height = (uint32_t)fb->height;
  fb_info.bpp = (uint32_t)fb->bpp;
  fb_info.pitch = (uint32_t)fb->pitch;

  fb_info.masks.red_size = fb->red_mask_size;
  fb_info.masks.red_shift = fb->red_mask_shift;
  fb_info.masks.green_size = fb->green_mask_size;
  fb_info.masks.green_shift = fb->green_mask_shift;
  fb_info.masks.blue_size = fb->blue_mask_size;
  fb_info.masks.blue_shift = fb->blue_mask_shift;

  // init flanterm
  // clang-format off
  fb_info.ft_ctx = flanterm_fb_init(
      NULL, NULL, fb_info.addr, 
      fb_info.width, fb_info.height, fb_info.pitch,
      fb_info.masks.red_size, fb_info.masks.red_shift, 
      fb_info.masks.green_size,
      fb_info.masks.green_shift, 
      fb_info.masks.blue_size, fb_info.masks.blue_shift, 
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      0, 0, 1, 0, 0, 0, 0);
  // clang-format on
}

// queue
static char term_buffer[4096] = {'\0'};
static size_t term_buffer_index = 0;

void framebuffer_write(const char str[]) {
  if (fb_info.ft_ctx == NULL) {
    size_t len = strlen(str);
    if (term_buffer_index + len >= sizeof(term_buffer)) {
      // overflow, ignore.
      return;
    }

    for (size_t i = 0; i < len; i++) {
      term_buffer[term_buffer_index++] = str[i];
    }
    term_buffer[term_buffer_index] = '\0';

    return;
  }

  if (term_buffer_index > 0) {
    // flush buffer
    flanterm_write(fb_info.ft_ctx, term_buffer, term_buffer_index);
    term_buffer_index = 0;
    term_buffer[0] = '\0';
  }

  flanterm_write(fb_info.ft_ctx, str, strlen(str));
}