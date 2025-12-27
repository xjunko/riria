#include <system.h>

int print(const char* msg) {
  long fd = 1;
  long len = 0;
  while (msg[len] != '\0') {
    len++;
  }
  return syscall3(1, fd, (long)msg, len);
}