#include <riria/fs/devfs.h>
#include <riria/libc.h>
#include <riria/types.h>

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

size_t devfs_read(const char* path, void* buffer, size_t sz) {
  if (!path) return -1;
  if (!buffer) return -2;

  devfs_dev_t* dev = _devfs_find_dev(path);
  if (!dev) return -3;
  if (!dev->read) return -4;

  size_t bytes_read = dev->read(path, buffer, sz);
  return bytes_read;
}

size_t devfs_write(const char* path, const void* buffer, size_t sz) {
  if (!path) return -1;
  if (!buffer) return -2;

  devfs_dev_t* dev = _devfs_find_dev(path);
  if (!dev) return -3;
  if (!dev->write) return -4;

  size_t bytes_written = dev->write(path, buffer, sz);
  return bytes_written;
}

bool devfs_exists(const char* path) {
  if (!path) return 0;

  devfs_dev_t* dev = _devfs_find_dev(path);
  if (dev) return 1;

  return 0;
}

void devfs_register_dev(const char* loc, fs_read read, fs_write write) {
  devfs_dev_t* dev = (devfs_dev_t*)malloc(sizeof(devfs_dev_t));
  if (!dev) {
    kprintf("[devfs] failed to allocate memory for device %s\n", loc);
    return;
  }

  dev->loc = loc;
  dev->read = read;
  dev->write = write;
  dev->next = NULL;

  if (!head) {
    head = dev;
    tail = dev;
  } else {
    tail->next = dev;
    tail = dev;
  }
  kprintf(", new_dev=%s", loc);
}

void devfs_init(vfs_impl_t* impl) {
  kprintf(", devfs INIT");
  impl->read = devfs_read;
  impl->write = devfs_write;
  impl->exists = devfs_exists;

  devfs_zero_install();
  devfs_stdout_install();
  kprintf(" OK, ");
}
