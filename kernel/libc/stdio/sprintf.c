#include <stdarg.h>
#include <stdio.h>

#include "nanoprintf.h"

// for now, it expects a buffer of at least 1024 bytes
int sprintf(char* buf, const char* fmt, ...) {
  int ret;

  va_list args;
  va_start(args, fmt);
  ret = npf_vsnprintf(buf, 1024, fmt, args);
  va_end(args);

  return ret;
}