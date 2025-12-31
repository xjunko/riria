#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define PCM_BUFFER 128000

static long syscall(int syscall_type, long a) {
  long ret;
  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"(syscall_type), "D"(a)
                   : "rcx", "r11", "memory");
  return ret;
}

int get_pit_cycles(void) { return (int)syscall(77, 0); }

void sleep_riria(uint32_t ms) {
  int start = get_pit_cycles();
  int end = start + ms;
  while (get_pit_cycles() < end) {
  }
}

int cat(const char* args) {
  const char* path = args;
  if (!path) {
    printf("no path specified.");
    return -1;
  }

  char path_to_file[256] = "/init/";
  strcat(path_to_file, path);

  FILE* f = fopen(path_to_file, "r");
  if (!f) {
    printf("failed to open %s\n", path_to_file);
    return -1;
  }

  char buffer[512];
  size_t bytes_read;

  while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
    fwrite(buffer, 1, bytes_read, stdout);
  }

  fclose(f);
  return 0;
}

int play_pcm(const char* args) {
  if (!args) {
    printf("no path specified.\n");
    return -1;
  }

  // build path safely
  char path_to_mus[256];
  int ret = snprintf(path_to_mus, sizeof(path_to_mus), "/init/audio/%s", args);
  if (ret < 0 || ret >= (int)sizeof(path_to_mus)) {
    printf("path too long!\n");
    return -1;
  }

  // open audio driver
  FILE* f_driver = fopen("/dev/audio", "w");
  if (!f_driver) {
    printf("failed to open audio driver\n");
    return -1;
  }
  setvbuf(f_driver, NULL, _IONBF, 0);  // unbuffered

  // open PCM file
  FILE* f_pcm = fopen(path_to_mus, "r");
  if (!f_pcm) {
    printf("failed to open PCM file %s\n", path_to_mus);
    fclose(f_driver);
    return -1;
  }
  setvbuf(f_pcm, NULL, _IONBF, 0);  // unbuffered

  // allocate heap buffer
  char* buffer = malloc(PCM_BUFFER);
  if (!buffer) {
    printf("failed to allocate audio buffer\n");
    fclose(f_driver);
    fclose(f_pcm);
    return -1;
  }

  long pcm_pos = 0;

  while (1) {
    size_t bytes_read = fread(buffer, 1, PCM_BUFFER, f_pcm);
    if (bytes_read == 0) break;  // EOF

    size_t total_written = 0;
    while (total_written < bytes_read) {
      size_t written = fwrite(buffer + total_written, 1,
                              bytes_read - total_written, f_driver);

      if (written == 0) {
        printf("audio driver write failed at byte %ld\n",
               pcm_pos + total_written);
        goto cleanup;
      }

      total_written += written;
      sleep_riria(512);  // we dont want the driver to be flooded
      printf("played %ld/%ld bytes\n", pcm_pos + total_written,
             pcm_pos + bytes_read);
    }
    pcm_pos += bytes_read;
  }

cleanup:
  free(buffer);
  fclose(f_driver);
  fclose(f_pcm);

  return 0;
}

int stress_test(void) {
  printf("starting memory stress test...\n");
  const size_t alloc_size = 10 * 1024 * 1024;  // 10 MB
  const int iterations = 10;

  for (int i = 0; i < iterations; i++) {
    printf("allocation %d/%d: allocating %zu bytes...\n", i + 1, iterations,
           alloc_size);
    void* ptr = malloc(alloc_size);
    if (!ptr) {
      printf("allocation failed!\n");
      return -1;
    }
    memset(ptr, 0xAA, alloc_size);  // touch the memory
    printf("allocation %d/%d: freeing memory...\n", i + 1, iterations);
    free(ptr);
  }

  printf("memory stress test completed successfully.\n");
  return 0;
}

int stress_heap_fragmentation(void) {
  printf("starting heap fragmentation stress test...\n");

  const int blocks = 10000;
  const size_t min_size = 16;
  const size_t max_size = 1024;

  void* ptrs[blocks];

  // allocate many small blocks
  for (int i = 0; i < blocks; i++) {
    size_t size = min_size + (i % (max_size - min_size));
    ptrs[i] = malloc(size);
    if (!ptrs[i]) {
      printf("allocation failed at %d\n", i);
      return -1;
    }
    memset(ptrs[i], 0xCC, size);
  }

  // free every other block
  for (int i = 0; i < blocks; i += 2) {
    free(ptrs[i]);
    ptrs[i] = NULL;
  }

  // allocate again to force reuse of holes
  for (int i = 0; i < blocks; i += 2) {
    ptrs[i] = malloc(512);
    if (!ptrs[i]) {
      printf("re-allocation failed at %d\n", i);
      return -1;
    }
    memset(ptrs[i], 0xDD, 512);
  }

  // cleanup
  for (int i = 0; i < blocks; i++) {
    free(ptrs[i]);
  }

  printf("heap fragmentation stress test completed.\n");
  return 0;
}

int stress_mmap(void) {
  printf("starting mmap stress test...\n");

  const int iterations = 1000;
  const size_t size = 4096 * 16;  // 64 KB

  for (int i = 0; i < iterations; i++) {
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (ptr == MAP_FAILED) {
      printf("mmap failed at iteration %d\n", i);
      return -1;
    }

    memset(ptr, 0xAB, size);

    if (munmap(ptr, size) != 0) {
      printf("munmap failed at iteration %d\n", i);
      return -1;
    }
  }

  printf("mmap stress test completed.\n");
  return 0;
}

int stress_file_io(void) {
  printf("starting file I/O stress test...\n");

  const int iterations = 1000;
  char buffer[1024];

  for (int i = 0; i < iterations; i++) {
    FILE* f = fopen("/init/test.txt", "r");
    if (!f) {
      printf("failed to open file at iteration %d\n", i);
      return -1;
    }

    fread(buffer, 1, sizeof(buffer), f);
    fclose(f);
  }

  printf("file I/O stress test completed.\n");
  return 0;
}

static int recurse(int depth) {
  char junk[256];
  memset(junk, depth, sizeof(junk));

  if (depth <= 0) return 0;
  return recurse(depth - 1);
}

int stress_stack(void) {
  printf("starting stack stress test...\n");
  recurse(1024);
  printf("stack stress test completed.\n");
  return 0;
}

int stress_all(void) {
  stress_test();
  stress_heap_fragmentation();
  // stress_mmap(); // this doesnt work yet
  stress_file_io();
  stress_stack();
  return 0;
}

void exec(const char* arg) {
  // arg0 - cmd
  // arg1... - args
  const char* cmd = strtok((char*)arg, " ");
  const char* args = strtok(NULL, "");

  if (strcmp(cmd, "cat") == 0) {
    cat(args);
  }

  if (strcmp(cmd, "play") == 0) {
    play_pcm(args);
  }

  if (strcmp(cmd, "stress") == 0) {
    stress_test();
  }

  if (strcmp(cmd, "stress-heap") == 0) {
    stress_heap_fragmentation();
  }

  if (strcmp(cmd, "stress-mmap") == 0) {
    stress_mmap();
  }

  if (strcmp(cmd, "stress-io") == 0) {
    stress_file_io();
  }

  if (strcmp(cmd, "stress-stack") == 0) {
    stress_stack();
  }

  if (strcmp(cmd, "stress-all") == 0) {
    stress_all();
  }
}

char getch(void) {
  FILE* f_kb = fopen("/dev/keyboard", "r");
  if (!f_kb) return '\0';
  setvbuf(f_kb, NULL, _IONBF, 0);

  char key[1] = {'\0'};
  while (1) {
    int ret = fread(key, 1, 1, f_kb);
    if (key[0] != '\0') {
      fclose(f_kb);
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
int main(void) {
  char buffer[SHELL_BUFFER] = {'\0'};

  // dont need it to be buffered
  setvbuf(stdout, NULL, _IONBF, 0);

  while (1) {
    printf("> ");

    size_t len = read_line(buffer, SHELL_BUFFER);
    memset(buffer + len, 0, SHELL_BUFFER - len);
    if (len == 0) continue;

    exec(buffer);
  }

  return 0;
}