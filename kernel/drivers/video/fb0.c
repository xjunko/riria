#include <riria/boot.h>
#include <riria/framebuffer.h>
#include <riria/libk.h>
#include <riria/mem.h>
#include <riria/types.h>

static struct {
  void* addr;
  void* buffer;
  uint32_t width;
  uint32_t height;
  uint32_t bpp;
  uint32_t pitch;
} fb_info __attribute__((used));

void framebuffer_install(uint32_t mb_info) { printk("[ fb] stub"); }