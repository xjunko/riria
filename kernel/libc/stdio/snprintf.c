#include <riria/libc.h>

int snprintf(char* buf, size_t size, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  npf_vsnprintf(buf, size, fmt, args);
  va_end(args);
  return 0;
}
