#include <riria/boot.h>
#include <riria/drivers/framebuffer.h>
#include <riria/fs/devfs.h>
#include <riria/fs/vfs.h>
#include <riria/types.h>
#include <string.h>

ssize_t _framebuffer_write(const char* path, const void* buffer, size_t sz) {
  UNUSED(path);
  UNUSED(buffer);
  UNUSED(sz);
  return sz;
}

void* _framebuffer_mmap(const char* path, size_t* len, int prot, int flags) {
  UNUSED(path);
  UNUSED(prot);
  UNUSED(flags);

  fb_info_t* fb = &fb_info;

  if (!fb) {
    return NULL;
  }

  *len = fb->pitch * fb->height;
  uint64_t phys_addr = (uint64_t)fb->addr - hhdm_request.response->offset;
  return (void*)phys_addr;
}

void devfs_framebuffer_install(void) {
  devfs_register_dev("/fb0", NULL, _framebuffer_write, _framebuffer_mmap);
}