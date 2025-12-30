#include <riria/drivers/ac97.h>
#include <riria/fs/devfs.h>
#include <riria/fs/vfs.h>
#include <riria/types.h>
#include <string.h>

ssize_t _audio_write(const char* path, const void* buffer, size_t sz) {
  UNUSED(path);

  while (!ac97_can_write())
    ;

  int wb = ac97_write_pcm((uint8_t*)buffer, sz);

  return wb;
}

void devfs_audio_install(void) {
  devfs_register_dev("/audio", NULL, _audio_write, NULL);
}