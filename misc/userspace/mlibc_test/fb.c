#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

// res=1024x768
// bpp=32
// pitch = 4096
#define FB_WIDTH 1024
#define FB_HEIGHT 768
#define FB_PITCH 4096

int main(void) {
  printf("Trying to mmap /dev/fb0...\n");
  // try mmaping the /dev/fb0
  int f_fb = open("/dev/fb0", O_RDWR);
  if (f_fb < 0) {
    printf("failed to open /dev/fb0\n");
    return -1;
  }

  uint8_t* fbPtr = mmap(NULL, FB_PITCH * FB_HEIGHT, PROT_READ | PROT_WRITE,
                        MAP_SHARED, f_fb, 0);

  printf("mmap returned fbPtr=%p\n", fbPtr);

  if (!fbPtr) {
    printf("failed to mmap /dev/fb0\n");
    close(f_fb);
    return -1;
  }

  // draw minecraft
  uint32_t i = 0;
  while (1) {
    i++;
    for (uint32_t x = (i % 640) + 100; x < (i % 640) + 200; x++) {
      for (uint32_t y = (i % 400) + 100; y < (i % 400) + 200; y++) {
        *((uint32_t*)fbPtr + y * (4096 >> 2) + x) = (i << 16) | (i << 8) | i;
      }
    }
  }
}