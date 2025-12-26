#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <system.h>

void _start(void) {
  print("hello from userspace and riria-libc!\n");
  exit(0);
}
