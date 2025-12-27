#include <riria/fs/vfs.h>
#include <riria/types.h>

int vfs_sys_open(const char* path, int flags, int mode) {
  vfs_file_t* file = vfs_open(path, flags, mode);
  if (!file) return -1;
  return file->id;
}

int vfs_sys_read(int fd, void* buffer, size_t sz) {
  vfs_file_t* file = vfs_get_from_fd(fd);
  return vfs_read(file, buffer, sz);
}

int vfs_sys_write(int fd, const void* buffer, size_t sz) {
  vfs_file_t* file = vfs_get_from_fd(fd);
  return vfs_write(file, buffer, sz);
}

int vfs_sys_seek(int fd, size_t offset, int whence) {
  vfs_file_t* file = vfs_get_from_fd(fd);
  return vfs_seek(file, offset, whence);
}

int vfs_sys_close(int fd) {
  vfs_file_t* file = vfs_get_from_fd(fd);
  return vfs_close(file);
}