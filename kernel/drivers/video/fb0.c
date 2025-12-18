#include <riria/boot.h>
#include <riria/framebuffer.h>
#include <riria/libk.h>
#include <riria/mem.h>
#include <stddef.h>
#include <stdint.h>

static struct {
  void* addr;
  void* buffer;
  uint32_t width;
  uint32_t height;
  uint32_t bpp;
  uint32_t pitch;
} fb_info __attribute__((used));

static void _mb2_parse_framebuffer(void* tag) {
  struct multiboot_tag_framebuffer* fb_tag =
      (struct multiboot_tag_framebuffer*)tag;

  fb_info.addr = (void*)(uintptr_t)fb_tag->common.framebuffer_addr;
  fb_info.width = fb_tag->common.framebuffer_width;
  fb_info.height = fb_tag->common.framebuffer_height;
  fb_info.bpp = fb_tag->common.framebuffer_bpp;
  fb_info.pitch = fb_tag->common.framebuffer_pitch;

  printk("[ fb] w=%d h=%d bpp=%d addr=0x%x\n", fb_info.width, fb_info.height,
         fb_info.bpp, fb_info.addr);
}

void framebuffer_install(uint32_t mb_info) {
  mb2_parse(mb_info, MULTIBOOT_TAG_TYPE_FRAMEBUFFER, _mb2_parse_framebuffer);

  // clear framebuffer with colors
  if (fb_info.addr && fb_info.bpp == 32) {
    uint32_t* pix = (uint32_t*)fb_info.addr;
    for (uint32_t y = 0; y < fb_info.height; y++) {
      for (uint32_t x = 0; x < fb_info.width; x++) {
        // color based on position
        pix[y * (fb_info.pitch / 4) + x] =
            0xFFFF0000 | ((x & 0xFF) << 16) | ((y & 0xFF) << 8);
      }
    }
  }
}