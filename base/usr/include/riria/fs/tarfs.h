#pragma once
#include <riria/fs/vfs.h>

typedef struct tarfs_file {
  const char* loc;
  struct tarfs_file* next;

  fs_read read;
  fs_write write;
} tarfs_file_t;

void tarfs_init(vfs_impl_t*);
ssize_t tarfs_read(const char*, void*, size_t);