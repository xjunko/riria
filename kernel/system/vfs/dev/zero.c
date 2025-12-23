#include <riria/fs/devfs.h>
#include <riria/fs/vfs.h>
#include <riria/types.h>

size_t _zero_read(const char* path, void* buffer, size_t sz) {
  memset(buffer, 0, sz);
  return sz;
}

size_t _zero_write(const char* path, const void* buffer, size_t sz) {
  return sz;
}

void devfs_zero_install(void) {
  devfs_register_dev("/zero", _zero_read, _zero_write);
}