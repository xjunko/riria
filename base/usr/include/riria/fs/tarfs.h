#pragma once
#include <riria/fs/vfs.h>

typedef struct tarfs_file {
  const char* loc;
  int seek_pos;
  size_t len;
  struct tarfs_file* next;

  fs_read read;
  fs_write write;
  fs_seek seek;
} tarfs_file_t;

void tarfs_init(vfs_impl_t*);
ssize_t tarfs_read(const char*, void*, size_t);
ssize_t tarfs_seek(const char*, size_t, int);
ssize_t tarfs_stat(const char*, vfs_file_stat_t*);
bool tarfs_exists(const char*);