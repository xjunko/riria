#include <riria/drivers/framebuffer.h>
#include <riria/drivers/serial.h>
#include <riria/libc.h>

#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_ALT_FORM_FLAG 0
#include "nanoprintf.h"

#define MAX_PRINTK 512

int printf(const char* fmt, ...) {
  char buf[1024] = {'\0'};
  va_list args;
  va_start(args, fmt);
  npf_vsnprintf(buf, 1024, fmt, args);
  va_end(args);

  for (int i = 0; buf[i]; i++) serial_write(buf[i]);
  framebuffer_write(buf);

  return 0;
}