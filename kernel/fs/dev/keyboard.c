#include <riria/drivers/ps2.h>
#include <riria/fs/devfs.h>
#include <riria/fs/vfs.h>
#include <riria/types.h>
#include <string.h>

KB_DRIVER_USER();

ssize_t _keyboard_read(const char* path, void* buffer, size_t sz) {
  UNUSED(path);

  for (size_t i = 0; i < sz; i++) {
    ((char*)buffer)[i] = KB_POP();
  }

  return sz;
}

ssize_t _keyboard_write(const char* path, const void* buffer, size_t sz) {
  UNUSED(path);
  UNUSED(buffer);
  return sz;
}

void devfs_keyboard_install(void) {
  devfs_register_dev("/keyboard", _keyboard_read, _keyboard_write);
}