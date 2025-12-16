#include <riria/libk.h>
#include <riria/serial.h>
#include <stdarg.h>
#include <stdint.h>

#define MAX_PRINTK 512

void printk(const char* fmt, ...) {
  char buf[MAX_PRINTK];
  va_list args;
  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  for (int i = 0; buf[i]; i++) serial_write(buf[i]);
}