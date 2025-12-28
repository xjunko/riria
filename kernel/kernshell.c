#include <riria/elf.h>
#include <riria/fs/devfs.h>
#include <riria/process.h>
#include <riria/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int starts_with(const char* str, const char* prefix) {
  size_t len_prefix = strlen(prefix);
  return strncmp(str, prefix, len_prefix) == 0;
}

void _test_elf(void) {
  vfs_file_t* elf_file = vfs_open("/init/test.elf", 0, 0);
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

void _test_bin(void) {
  vfs_file_t* bin_file = vfs_open("/init/test.bin", 0, 0);
  if (!bin_file) {
    printf("failed: open \n");
    return;
  }

  vfs_file_stat_t stat;
  int res = vfs_stat(bin_file, &stat);
  if (res < 0) {
    printf("failed: stat");
    return;
  }

  void* buf = malloc(stat.size);
  if (!buf) {
    printf("failed: malloc");
    return;
  }

  int bytes_read = vfs_read(bin_file, buf, stat.size);
  if (bytes_read < 0) {
    printf("failed: read \n");
    return;
  }

  int ret = vfs_close(bin_file);
  if (ret < 0) {
    printf("failed: close \n");
    return;
  }

  process_spawn_user(buf, stat.size, USER_VIRT_START);
}

void _mus_play_proc(void) {
  printf("path: cyberfantasia.pcm \n");

  printf("driver::get ->");
  vfs_file_t* driver_file = vfs_open("/dev/audio", 0, 0);
  if (!driver_file) {
    printf(" FAILED\n");
    return;
  }
  printf(" OK\n");

  printf("file::get ->");
  vfs_file_t* mus_file = vfs_open("/init/cyberfantasia.pcm", 0, 0);
  if (!mus_file) {
    printf(" FAILED\n");
    return;
  }
  printf(" OK\n");

  printf("playback::setup ->");
  // start playing
  size_t chunk_size = 128000;
  void* buffer = malloc(chunk_size);
  int pcm_pos = 0;
  printf(" OK\n");

  printf("playback::playing -> yes\n");

  for (;;) {
    vfs_seek(mus_file, pcm_pos, 0);
    int bytes_read = vfs_read(mus_file, buffer, chunk_size);
    ASSERT(bytes_read >= 0);

    if (bytes_read == 0) break;

    int total_written = 0;
    while (total_written < bytes_read) {
      int written = vfs_write(driver_file, (uint8_t*)buffer + total_written,
                              bytes_read - total_written);
      total_written += written;
    }
    pcm_pos += bytes_read;
  }
  int ret = vfs_close(mus_file);
  ASSERT(ret >= 0);
  ret = vfs_close(driver_file);
  ASSERT(ret >= 0);
  free(buffer);
  printf("exit: done playing music\n");
}

void _run_quickly_then_die(void) { printf("hello world\n"); }

void exec(const char* arg) {
  printf("> exec(%s)\n", arg);
  if (starts_with(arg, "play ")) {
    process_create(_mus_play_proc, NULL);
  }

  if (strcmp(arg, "test") == 0) {
    process_create(_test_bin, NULL);
  }

  if (strcmp(arg, "elf") == 0) {
    process_create(_test_elf, NULL);
  }

  if (strcmp(arg, "quick") == 0) {
    process_create(_run_quickly_then_die, NULL);
  }
}

char getch(void) {
  vfs_file_t* keyboard_file = vfs_open("/dev/keyboard", 0, 0);
  ASSERT(keyboard_file);

  char key[1] = {'\0'};
  while (1) {
    int ret = vfs_read(keyboard_file, key, 1);
    ASSERT(ret >= 0);
    if (key[0] != '\0') {
      vfs_close(keyboard_file);
      return key[0];
    }
  }
}

size_t read_line(char* buf, size_t len) {
  size_t pos = 0;

  while (1) {
    char c = getch();

    switch (c) {
      case '\n':
        printf("\n");
        return pos;
        break;
      case '\b':
        if (pos > 0) {
          pos -= 1;
          printf("\b \b");
        }
        break;
      case 32 ... 126:
        if (pos < len - 1) {
          buf[pos] = c;
          pos++;
          printf("%c", c);
        }
        break;
      default:
        break;
    }
  }
}

#define SHELL_BUFFER 128
void kernel_shell(void) {
  char buffer[SHELL_BUFFER] = {'\0'};

  while (1) {
    printf("> ");

    size_t len = read_line(buffer, SHELL_BUFFER);
    memset(buffer + len, 0, SHELL_BUFFER - len);
    if (len == 0) continue;

    exec(buffer);
  }
}