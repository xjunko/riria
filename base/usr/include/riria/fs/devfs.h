#pragma once
#include <riria/fs/vfs.h>
#include <riria/types.h>

typedef struct devfs_dev {
  const char* loc;
  struct devfs_dev* next;

  fs_read read;
  fs_write write;
} devfs_dev_t;

void devfs_init(vfs_impl_t*);
size_t devfs_read(const char*, void*, size_t);
size_t devfs_write(const char*, const void*, size_t);
bool devfs_exists(const char*);
void devfs_register_dev(const char*, fs_read, fs_write);

// /dev/zero
void devfs_zero_install(void);

// /dev/stdout
void devfs_stdout_install(void);