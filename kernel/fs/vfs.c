#include <riria/fs/devfs.h>
#include <riria/fs/tarfs.h>
#include <riria/fs/vfs.h>
#include <riria/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static vfs_t mountpoints[VFS_MAX_MOUNTPOINTS];
static vfs_file_t* open_files[VFS_MAX_FILE_DESCRIPTORS];

static bool _startswith(const char* str, const char* prefix) {
  size_t prefix_len = strlen(prefix);
  return strncmp(str, prefix, prefix_len) == 0;
}

static vfs_t* _vfs_find_mnt(const char* path) {
  if (!path) return NULL;
  for (int i = 0; i < VFS_MAX_MOUNTPOINTS; i++) {
    if (mountpoints[i].mnt) {
      if (_startswith(path, mountpoints[i].mnt)) {
        return &mountpoints[i];
      }
    }
  }
  return NULL;
}

static int _vfs_get_fd(vfs_file_t* file) {
  if (!file) return -1;

  for (int i = VFS_FD_OFFSET; i < VFS_MAX_FILE_DESCRIPTORS; i++) {
    if (!open_files[i]) {
      open_files[i] = file;
      file->id = i;
      return i;
    }
  }

  return -1;
}

vfs_file_t* vfs_get_from_fd(int fd) {
  if (fd < 0 || fd >= VFS_MAX_FILE_DESCRIPTORS || !open_files[fd]) {
    return NULL;
  }
  return open_files[fd];
}

vfs_file_t* vfs_open(const char* path, int flags, int mode) {
  if (!path) return NULL;

  vfs_t* mnt = _vfs_find_mnt(path);
  if (!mnt) return NULL;
  if (!mnt->impl) return NULL;
  if (!mnt->impl->exists) return NULL;

  const char* loc_wo_mnt = path + strlen(mnt->mnt) - 1;
  vfs_file_t* file = malloc(sizeof(vfs_file_t));
  if (!file) return NULL;
  if (mnt->impl->exists(loc_wo_mnt) == 0) {
    free(file);
    return NULL;
  }

  file->id = _vfs_get_fd(file);
  file->loc = path;
  file->flags = flags;
  file->mode = mode;
  file->fs = mnt;

  open_files[file->id] = file;

  return file;
}

int vfs_read(vfs_file_t* file, void* buffer, size_t sz) {
  if (!file) return -1;
  if (!file->loc) return -2;
  if (!file->fs) return -3;
  if (!file->fs->impl) return -4;
  if (!file->fs->impl->read) return 0;

  const char* loc_wo_mnt = file->loc + strlen(file->fs->mnt) - 1;
  return file->fs->impl->read(loc_wo_mnt, buffer, sz);
}

int vfs_write(vfs_file_t* file, const void* buffer, size_t sz) {
  if (!file) return -1;
  if (!file->loc) return -2;
  if (!file->fs) return -3;
  if (!file->fs->impl) return -4;
  if (!file->fs->impl->write) return 0;

  const char* loc_wo_mnt = file->loc + strlen(file->fs->mnt) - 1;
  return file->fs->impl->write(loc_wo_mnt, buffer, sz);
}

int vfs_seek(vfs_file_t* file, size_t offset, int whence) {
  if (!file) return -1;
  if (!file->loc) return -2;
  if (!file->fs) return -3;
  if (!file->fs->impl) return -4;
  if (!file->fs->impl->seek) return 0;

  const char* loc_wo_mnt = file->loc + strlen(file->fs->mnt) - 1;
  return file->fs->impl->seek(loc_wo_mnt, offset, whence);
}

int vfs_stat(vfs_file_t* file, vfs_file_stat_t* stat) {
  if (!file) return -1;
  if (!file->loc) return -2;
  if (!file->fs) return -3;
  if (!file->fs->impl) return -4;
  if (!file->fs->impl->stat) return 0;

  const char* loc_wo_mnt = file->loc + strlen(file->fs->mnt) - 1;
  return file->fs->impl->stat(loc_wo_mnt, stat);
}

int vfs_exists(vfs_file_t* file) {
  if (!file) return -1;
  if (!file->loc) return -2;
  if (!file->fs) return -3;

  return file->fs->impl->exists(file->loc) ? 1 : 0;
}

void* vfs_mmap(vfs_file_t* file, size_t* len, int prot, int flags) {
  if (!file) return NULL;
  if (!file->loc) return NULL;
  if (!file->fs) return NULL;
  if (!file->fs->impl) return NULL;
  if (!file->fs->impl->mmap) return NULL;

  const char* loc_wo_mnt = file->loc + strlen(file->fs->mnt) - 1;
  return file->fs->impl->mmap(loc_wo_mnt, len, prot, flags);
}

int vfs_close(vfs_file_t* file) {
  if (!file) return -1;
  if (file->id < 0 || file->id >= VFS_MAX_FILE_DESCRIPTORS ||
      !open_files[file->id]) {
    return -2;
  }

  open_files[file->id] = NULL;
  free(file);
  return 0;
}

void vfs_install(void) {
  printf(DEBUG "[   vfs] VFS INIT...");
  vfs_impl_t* devfs = malloc(sizeof(vfs_impl_t));
  mountpoints[0].mnt = "/dev/";
  mountpoints[0].impl = devfs;
  devfs_init(devfs);

  vfs_impl_t* tarfs = malloc(sizeof(vfs_impl_t));
  mountpoints[1].mnt = "/init/";
  mountpoints[1].impl = tarfs;
  tarfs_init(tarfs);

  printf(GREEN ", ALL OK!\n");
}