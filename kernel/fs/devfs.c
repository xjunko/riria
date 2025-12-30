#include <riria/fs/devfs.h>
#include <riria/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static devfs_dev_t* head = NULL;
static devfs_dev_t* tail = NULL;

static devfs_dev_t* _devfs_find_dev(const char* path) {
  devfs_dev_t* curr = head;
  while (curr) {
    if (strcmp(curr->loc, path) == 0) {
      return curr;
    }
    curr = curr->next;
  }

  return NULL;
}

ssize_t devfs_read(const char* path, void* buffer, size_t sz) {
  if (!path) return -1;
  if (!buffer) return -2;

  devfs_dev_t* dev = _devfs_find_dev(path);
  if (!dev) return -3;
  if (!dev->read) return -4;

  size_t bytes_read = dev->read(path, buffer, sz);
  return bytes_read;
}

ssize_t devfs_write(const char* path, const void* buffer, size_t sz) {
  if (!path) return -1;
  if (!buffer) return -2;

  devfs_dev_t* dev = _devfs_find_dev(path);
  if (!dev) return -3;
  if (!dev->write) return -4;

  size_t bytes_written = dev->write(path, buffer, sz);
  return bytes_written;
}

void* devfs_mmap(const char* path, size_t* len, int prot, int flags) {
  if (!path) return NULL;
  if (!len) return NULL;

  devfs_dev_t* dev = _devfs_find_dev(path);
  if (!dev) return NULL;
  if (!dev->mmap) return NULL;

  return dev->mmap(path, len, prot, flags);
}

bool devfs_exists(const char* path) {
  if (!path) return 0;

  devfs_dev_t* dev = _devfs_find_dev(path);
  if (dev) return 1;

  return 0;
}

void devfs_register_dev(const char* loc, fs_read read, fs_write write,
                        fs_mmap mmap) {
  devfs_dev_t* dev = (devfs_dev_t*)malloc(sizeof(devfs_dev_t));
  if (!dev) {
    printf("[devfs] failed to allocate memory for device %s\n", loc);
    return;
  }

  dev->loc = loc;
  dev->read = read;
  dev->write = write;
  dev->mmap = mmap;
  dev->next = NULL;

  if (!head) {
    head = dev;
    tail = dev;
  } else {
    tail->next = dev;
    tail = dev;
  }
  printf(", new_dev=%s", loc);
}

void devfs_init(vfs_impl_t* impl) {
  printf(", devfs INIT");
  impl->read = devfs_read;
  impl->write = devfs_write;
  impl->exists = devfs_exists;
  impl->seek = NULL;
  impl->stat = NULL;
  impl->mmap = devfs_mmap;

  devfs_zero_install();
  devfs_stdout_install();
  devfs_audio_install();
  devfs_keyboard_install();
  devfs_framebuffer_install();
  printf(" OK, ");
}
