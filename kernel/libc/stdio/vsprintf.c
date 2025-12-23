#include <riria/libc.h>
#include <stdarg.h>
#include <stdint.h>

static void reverse(char* str, int len) {
  int i = 0, j = len - 1;
  while (i < j) {
    char tmp = str[i];
    str[i] = str[j];
    str[j] = tmp;
    i++;
    j--;
  }
}

static int utoa(uint32_t value, char* str, int base) {
  int i = 0;

  if (value == 0) {
    str[i++] = '0';
    str[i] = '\0';
    return i;
  }

  while (value != 0) {
    uint32_t rem = value % base;
    str[i++] = (rem < 10) ? ('0' + rem) : ('a' + rem - 10);
    value /= base;
  }

  str[i] = '\0';
  reverse(str, i);
  return i;
}

static int itoa(int value, char* str, int base) {
  if (base != 10) {
    // unlikely to work with itoa, so use utoa for that
    return utoa((uint32_t)value, str, base);
  }

  int i = 0;
  int is_negative = 0;

  if (value == 0) {
    str[i++] = '0';
    str[i] = '\0';
    return i;
  }

  if (value < 0) {
    is_negative = 1;
    value = -value;
  }

  while (value != 0) {
    int rem = value % 10;
    str[i++] = '0' + rem;
    value /= 10;
  }

  if (is_negative) str[i++] = '-';

  str[i] = '\0';
  reverse(str, i);
  return i;
}

int vsprintf(char* buffer, const char* fmt, va_list args) {
  int i = 0;  // buffer index
  for (int f = 0; fmt[f]; f++) {
    if (fmt[f] == '%') {
      f++;
      char temp[32];
      switch (fmt[f]) {
        case 'd':
          itoa(va_arg(args, int), temp, 10);
          break;
        case 'x':
          itoa(va_arg(args, int), temp, 16);
          break;
        case 's': {
          char* s = va_arg(args, char*);
          int j = 0;
          while (s[j]) temp[j] = s[j], j++;
          temp[j] = '\0';
        } break;
        case '%':
          temp[0] = '%';
          temp[1] = '\0';
          break;
        default:
          temp[0] = '?';
          temp[1] = '\0';
          break;
      }
      // copy temp to buffer
      int j = 0;
      while (temp[j]) buffer[i++] = temp[j++];
    } else {
      buffer[i++] = fmt[f];
    }
  }
  buffer[i] = '\0';
  return i;
}
