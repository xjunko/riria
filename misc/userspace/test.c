#include <stddef.h>
#include <stdlib.h>
#include <system.h>

void _start(void) {
  int ret;
  int fd = 1;
  const char* msg = "hello from userspace!\n";
  size_t len = 23;
  syscall3(4, (long)fd, (long)msg, (long)len);
  exit(1);
}
