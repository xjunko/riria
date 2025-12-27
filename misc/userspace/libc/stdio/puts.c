#include <stdio.h>
#include <system.h>

int puts(const char* s) {
  while (*s) {
    if (*s == '\n') {
      syscall3(4, 1, (long)"\n", 1);
    } else {
      syscall3(4, 1, (long)s, 1);
    }
    s++;
  }
  return 0;
}