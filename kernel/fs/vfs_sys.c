#include <riria/fs/vfs.h>
#include <riria/types.h>

static inline int MLIBC_HACK(int ret) {
  if (ret < 0) {
    printf(WARNING "[ vfs_sys] MLIBC_HACK occured: %d to 0\n", ret);
    return 0;
  }
  return ret;
}

int vfs_sys_open(const char* path, int flags, int mode) {
  vfs_file_t* file = vfs_open(path, flags, mode);
  if (!file) return -1;
  return file->id;
}

int vfs_sys_read(int fd, void* buffer, size_t sz) {
  vfs_file_t* file = vfs_get_from_fd(fd);
  return MLIBC_HACK(vfs_read(file, buffer, sz));
}

int vfs_sys_write(int fd, const void* buffer, size_t sz) {
  vfs_file_t* file = vfs_get_from_fd(fd);
  return MLIBC_HACK(vfs_write(file, buffer, sz));
}

int vfs_sys_seek(int fd, size_t offset, int whence) {
  vfs_file_t* file = vfs_get_from_fd(fd);
  return MLIBC_HACK(vfs_seek(file, offset, whence));
}

void* vfs_sys_mmap(int fd, size_t* len, int prot, int flags) {
  vfs_file_t* file = vfs_get_from_fd(fd);
  return vfs_mmap(file, len, prot, flags);
}

int vfs_sys_close(int fd) {
  vfs_file_t* file = vfs_get_from_fd(fd);
  return vfs_close(file);
}