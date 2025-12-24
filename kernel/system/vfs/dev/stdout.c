#include <riria/drivers/serial.h>
#include <riria/fs/devfs.h>
#include <riria/fs/vfs.h>
#include <riria/types.h>

size_t _stdout_read(const char* path, void* buffer, size_t sz) {
  // noop, for now
  UNUSED(path);
  UNUSED(buffer);
  return sz;
}

size_t _stdout_write(const char* path, const void* buffer, size_t sz) {
  UNUSED(path);
  for (size_t i = 0; i < sz; i++) {
    serial_write(((const char*)buffer)[i]);
  }

  return sz;
}

void devfs_stdout_install(void) {
  devfs_register_dev("/stdout", _stdout_read, _stdout_write);
}