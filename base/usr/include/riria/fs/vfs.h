#pragma once
#include <riria/types.h>

#define VFS_MAX_MOUNTPOINTS 16
#define VFS_MAX_FILE_DESCRIPTORS 256
#define VFS_FD_OFFSET 10

typedef struct vfs vfs_t;
typedef struct vfs_impl vfs_impl_t;
typedef struct vfs_file vfs_file_t;
typedef struct vfs_file_stat vfs_file_stat_t;

typedef ssize_t (*fs_read)(const char* path, void* buffer, size_t sz);
typedef ssize_t (*fs_write)(const char* path, const void* buffer, size_t sz);
typedef ssize_t (*fs_seek)(const char* path, size_t offset, int whence);
typedef ssize_t (*fs_stat)(const char* path, vfs_file_stat_t* stat);
typedef void* (*fs_mmap)(const char* path, size_t* len, int prot, int flags);
typedef bool (*fs_exists)(const char* path);

// what we expect from any fs implementation
typedef struct vfs_impl {
  fs_read read;
  fs_write write;
  fs_seek seek;
  fs_exists exists;
  fs_stat stat;
  fs_mmap mmap;
} vfs_impl_t;

// public API below:
// file stats
typedef struct vfs_file_stat {
  int type;
  size_t size;
} vfs_file_stat_t;

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
int vfs_seek(vfs_file_t*, size_t, int);
int vfs_stat(vfs_file_t*, vfs_file_stat_t*);
void* vfs_mmap(vfs_file_t*, size_t*, int, int);
int vfs_close(vfs_file_t*);
int vfs_exists(vfs_file_t*);

// syscall layer
vfs_file_t* vfs_get_from_fd(int fd);
int vfs_sys_open(const char*, int, int);
int vfs_sys_read(int, void*, size_t);
int vfs_sys_write(int, const void*, size_t);
int vfs_sys_seek(int, size_t, int);
void* vfs_sys_mmap(int, size_t*, int, int);
int vfs_sys_close(int);