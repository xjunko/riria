#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <system.h>

#define PCM_BUFFER 128000
static char buf[PCM_BUFFER] = {0};

void _start(void) {
  // /dev/stdout
  long fd = syscall3(5, (long)"/dev/stdout", 0, 0);
  const char* msg = "writing this to /dev/stdout\n";
  syscall3(4, fd, (long)msg, 29);
  syscall1(6, fd);

  // /init/cyberfantasia.pcm
  long driver_fd = syscall3(5, (long)"/dev/audio", 0, 0);
  long pcm_fd = syscall3(5, (long)"/init/cyberfantasia.pcm", 0, 0);
  long pcm_pos = 0;

  for (;;) {
    syscall3(8, pcm_fd, pcm_pos, 0);

    long bytes_read = syscall3(3, pcm_fd, (long)buf, PCM_BUFFER);
    if (bytes_read <= 0) break;

    long total_written = 0;
    while (total_written < bytes_read) {
      long written = syscall3(4, driver_fd, (long)buf, PCM_BUFFER);
      total_written += written;
    }

    pcm_pos += bytes_read;
  }

  puts("hello world, this is from the userspace, using the userspace libc!\n");
  exit(1);
}
