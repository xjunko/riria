#pragma once
#include <riria/types.h>

#define VFS_MAX_MOUNTPOINTS 16
#define VFS_MAX_FILE_DESCRIPTORS 256
#define VFS_FD_OFFSET 10

typedef ssize_t (*fs_read)(const char* path, void* buffer, size_t sz);
typedef ssize_t (*fs_write)(const char* path, const void* buffer, size_t sz);
typedef bool (*fs_exists)(const char* path);

// what we expect from any fs implementation
typedef struct vfs_impl {
  fs_read read;
  fs_write write;
  fs_exists exists;
} vfs_impl_t;

// FIXME: make this unix-like or atleast sane, current design is garbage
typedef struct vfs_file {
  int id;
  const char* loc;
  int flags;
  int mode;

  struct vfs* fs;
} vfs_file_t;

typedef struct vfs {
  const char* mnt;
  vfs_impl_t* impl;
} vfs_t;

void vfs_install(void);
vfs_file_t* vfs_open(const char*, int, int);
int vfs_read(vfs_file_t*, void*, size_t);
int vfs_write(vfs_file_t*, const void*, size_t);
int vfs_close(vfs_file_t*);
int vfs_exists(vfs_file_t*);