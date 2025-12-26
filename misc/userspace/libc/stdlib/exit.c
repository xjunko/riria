#include <stdlib.h>
#include <system.h>

void exit(int status) {
  syscall1(1, (long)status);
  for (;;)
    ;
}