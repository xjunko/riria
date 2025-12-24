#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "nanoprintf.h"

int vsprintf(char* buffer, const char* fmt, va_list args) {
  return npf_vsnprintf(buffer, SIZE_MAX, fmt, args);
}
