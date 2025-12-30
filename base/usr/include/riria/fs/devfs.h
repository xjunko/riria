#pragma once
#include <riria/fs/vfs.h>
#include <riria/types.h>

typedef struct devfs_dev {
  const char* loc;
  struct devfs_dev* next;

  fs_read read;
  fs_write write;
  fs_mmap mmap;
} devfs_dev_t;

void devfs_init(vfs_impl_t*);
ssize_t devfs_read(const char*, void*, size_t);
ssize_t devfs_write(const char*, const void*, size_t);
void* devfs_mmap(const char*, size_t*, int, int);
bool devfs_exists(const char*);
void devfs_register_dev(const char*, fs_read, fs_write, fs_mmap);

// /dev/zero
void devfs_zero_install(void);

// /dev/stdout
void devfs_stdout_install(void);

// /dev/audio
void devfs_audio_install(void);

// /dev/keyboard
void devfs_keyboard_install(void);

// /dev/fb0
void devfs_framebuffer_install(void);