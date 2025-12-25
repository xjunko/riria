#include <riria/boot.h>
#include <riria/fs/devfs.h>
#include <riria/fs/tarfs.h>
#include <riria/fs/vfs.h>
#include <riria/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void* initramfs;
tarfs_file_t* tarfs_head = NULL;
tarfs_file_t* tarfs_tail = NULL;

static void _tarfs_add_file(const char* loc) {
  tarfs_file_t* file = malloc(sizeof(tarfs_file_t));
  ASSERT(file);

  // ./file.ext to /file.ext
  const char* loc_wo_dot = loc + 1;

  file->loc = loc_wo_dot;
  file->read = tarfs_read;
  file->write = NULL;
  file->seek = tarfs_seek;
  file->next = NULL;

  if (!tarfs_head) {
    tarfs_head = file;
    tarfs_tail = file;
  } else {
    tarfs_tail->next = file;
    tarfs_tail = file;
  }

  printf(", tarfs_file=%s", file->loc);
}

static tarfs_file_t* _tarfs_find_file(const char* path) {
  tarfs_file_t* curr = tarfs_head;
  while (curr) {
    if (strcmp(curr->loc, path) == 0) {
      return curr;
    }
    curr = curr->next;
  }

  printf("tarfs: file not found: %s\n", path);
  UNREACHABLE();

  return NULL;
}

static int oct2bin(unsigned char* str, int size) {
  int n = 0;
  unsigned char* c = str;
  while (size-- > 0) {
    n *= 8;
    n += *c - '0';
    c++;
  }
  return n;
}

ssize_t tarfs_read(const char* path, void* buffer, size_t sz) {
  char tar_path[256];
  tar_path[0] = '.';
  strcpy(tar_path + 1, path);

  unsigned char* ptr = (unsigned char*)initramfs;
  while (!memcmp(ptr + 257, "ustar", 5)) {
    int file_size = oct2bin(ptr + 0x7c, 11);
    char* path = (char*)ptr;

    if (!memcmp(ptr, tar_path, strlen(tar_path) + 1)) {
      // NOTE: this is fairly hacky, but it works for now
      tarfs_file_t* file = _tarfs_find_file(tar_path + 1);
      ASSERT(file);

      // HACK: oooh spooky
      if (file->seek_pos != 0) {
        file_size -= file->seek_pos;
        ptr += 512 + file->seek_pos;
      }

      unsigned char* data = ptr + 512;
      size_t bytes_to_read = (file_size < (int)sz) ? file_size : (int)sz;

      for (size_t i = 0; i < bytes_to_read; i++) {
        ((unsigned char*)buffer)[i] = data[i];
      }

      return bytes_to_read;
    }

    ptr += (((file_size + 511) / 512) + 1) * 512;
  }

  return -1;
}

ssize_t tarfs_seek(const char* path, size_t offset, int whence) {
  if (!path) return -1;

  // TODO: use whence
  UNUSED(whence);

  tarfs_file_t* file = _tarfs_find_file(path);
  if (!file) return -1;
  file->seek_pos = offset;
  return file->seek_pos;
}

bool tarfs_exists(const char* path) {
  if (!path) return 0;

  tarfs_file_t* file = _tarfs_find_file(path);
  if (file) return 1;
  return 0;
}

void tarfs_init(vfs_impl_t* impl) {
  impl->read = tarfs_read;
  impl->write = NULL;
  impl->exists = tarfs_exists;
  impl->seek = tarfs_seek;

  for (uint64_t i = 0; i < module_request.response->module_count; i++) {
    if (strcmp(module_request.response->modules[i]->path,
               "/boot/initramfs.tar") == 0) {
      initramfs = module_request.response->modules[i]->address;
      break;
    }
  }

  if (!initramfs) {
    panic("no initramfs provided!");
  }

  unsigned char* ptr = (unsigned char*)initramfs;
  while (!memcmp(ptr + 257, "ustar", 5)) {
    int file_size = oct2bin(ptr + 0x7c, 11);
    char* path = (char*)ptr;

    if (file_size > 0) {
      _tarfs_add_file(path);
    }

    ptr += (((file_size + 511) / 512) + 1) * 512;
  }

  printf(", tarfs INIT");
}