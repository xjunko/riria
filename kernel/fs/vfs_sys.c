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
  // HACK: mlibc straight up errors out if /stdout if the ret is negative
  int ret = vfs_seek(file, offset, whence);
  if (fd == 1) {
    return ret < 0 ? 0 : ret;
  }
  return ret;
}

void* vfs_sys_mmap(int fd, size_t* len, int prot, int flags) {
  vfs_file_t* file = vfs_get_from_fd(fd);
  return vfs_mmap(file, len, prot, flags);
}

int vfs_sys_close(int fd) {
  vfs_file_t* file = vfs_get_from_fd(fd);
  return vfs_close(file);
}