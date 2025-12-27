#include <system.h>

int printf(const char* fmt, ...) {
  UNUSED(fmt);

  long fd = 1;
  const char* msg = "call to printf() is not yet implemented\n";
  long len = 41;
  return syscall3(4, fd, (long)msg, len);
}