#include <riria/elf.h>
#include <riria/fs/devfs.h>
#include <riria/process.h>
#include <riria/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void run_elf(const char* path) {
  vfs_file_t* elf_file = vfs_open(path, 0, 0);
  if (!elf_file) {
    printf("failed: open \n");
    return;
  }

  vfs_file_stat_t stat;
  int res = vfs_stat(elf_file, &stat);
  if (res < 0) {
    printf("failed: stat");
    return;
  }

  void* buf = malloc(stat.size);
  if (!buf) {
    printf("failed: malloc");
    return;
  }

  int bytes_read = vfs_read(elf_file, buf, stat.size);
  if (bytes_read < 0) {
    printf("failed: read \n");
    return;
  }

  int ret = vfs_close(elf_file);
  if (ret < 0) {
    printf("failed: close \n");
    return;
  }

  process_spawn_elf(buf, stat.size);
}

void userspace_shell(void) {
  run_elf("/init/doomgeneric");
  // run_elf("/init/shell.elf");
}