#include <riria/fs/devfs.h>
#include <riria/fs/vfs.h>
#include <riria/types.h>
#include <string.h>

ssize_t _zero_read(const char* path, void* buffer, size_t sz) {
  UNUSED(path);
  memset(buffer, 0, sz);
  return sz;
}

ssize_t _zero_write(const char* path, const void* buffer, size_t sz) {
  UNUSED(path);
  UNUSED(buffer);
  return sz;
}

void devfs_zero_install(void) {
  devfs_register_dev("/zero", _zero_read, _zero_write, NULL);
}