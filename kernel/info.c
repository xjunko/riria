#include <riria/cpu/irq.h>
#include <riria/drivers/framebuffer.h>
#include <riria/fs/vfs.h>
#include <riria/mem.h>
#include <riria/types.h>
#include <stdlib.h>
#include <unistd.h>

#define SSFN_CONSOLEBITMAP_TRUECOLOR
#include <riria/thirdparty/ssfn.h>

void kernel_info(void) {
  vfs_file_t* font_file = vfs_open("/init/fonts/lanapixel.sfn", 0, 0);
  if (!font_file) {
    printf(ERROR
           "[  info] failed to open lanapixel.sfn, no kernel info will be "
           "drawn.\n");
    return;
  }

  vfs_file_stat_t font_stat;
  if (vfs_stat(font_file, &font_stat) != 0) {
    printf(ERROR
           "[  info] failed to stat lanapixel.sfn, no kernel info will be "
           "drawn.\n");
    vfs_close(font_file);
    return;
  }

  size_t font_size = font_stat.size;
  uint8_t* font_buffer = malloc(font_size);
  if (!font_buffer) {
    printf(ERROR
           "[  info] failed to allocate memory for lanapixel.sfn, no kernel "
           "info will be drawn.\n");
    vfs_close(font_file);
    return;
  }

  ssize_t read_bytes = vfs_read(font_file, font_buffer, font_size);
  if (read_bytes != (ssize_t)font_size) {
    printf(ERROR
           "[  info] failed to read lanapixel.sfn, no kernel info will be "
           "drawn.\n");
    free(font_buffer);
    vfs_close(font_file);
    return;
  }
  vfs_close(font_file);

  // should be fine now...
  ssfn_src = (ssfn_font_t*)font_buffer;
  ssfn_dst.ptr = fb_info.addr;
  ssfn_dst.p = fb_info.pitch;
  ssfn_dst.fg = 0xFFFFFFFF;
  ssfn_dst.bg = 0x0;
  ssfn_dst.x = 850;
  ssfn_dst.y = 100;

  char info_buffer[1024];
  while (1) {
    sprintf(info_buffer,
            "-------------------\n"
            "Total RAM: %u MB\n"
            "Free RAM: %u MB\n"
            "Framebuffer: %ux%u @ %u bpp\n"
            "Ticks: %u\n"
            "-------------------\n",
            (uint32_t)((pmm_get_total_pages() * 4096) / (1024 * 1024)),
            (uint32_t)((pmm_get_free_pages() * 4096) / (1024 * 1024)),
            fb_info.width, fb_info.height, fb_info.bpp, ticks);

    // might as well reset the area around the text
    for (size_t y = 100; y < 300; y++) {
      for (size_t x = 850; x < 1150; x++) {
        framebuffer_draw_pixel(x, y, 0x000000);
      }
    }

    ssfn_dst.x = 850;
    ssfn_dst.y = 100;
    for (size_t i = 0; info_buffer[i] != '\0'; i++) {
      if (info_buffer[i] == '\n') {
        ssfn_dst.x = 850;
        ssfn_dst.y += ssfn_src->height;
        continue;
      }
      ssfn_putc(info_buffer[i]);
    }

    sleep(1);
  }
}