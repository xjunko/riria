#include <riria/boot.h>
#include <riria/drivers/framebuffer.h>
#include <riria/libc.h>
#include <riria/types.h>

static struct {
  void* addr;
  void* buffer;
  uint32_t width;
  uint32_t height;
  uint32_t bpp;
  uint32_t pitch;
} fb_info __attribute__((used));

void framebuffer_install(void) { printk("[ fb] stub"); }