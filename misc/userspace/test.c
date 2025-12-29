#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PCM_BUFFER 128000

int main(void) {
  // /dev/stdout
  FILE* f_stdout = fopen("/dev/stdout", "w");
  if (!f_stdout) {
    return -1;
  }

  const char* msg = "writing this to /dev/stdout\n";
  fwrite(msg, 1, strlen(msg), f_stdout);

  if (fclose(f_stdout) != 0) {
    return -1;
  }

  FILE* f_driver = fopen("/dev/audio", "w");
  FILE* f_pcm = fopen("/init/cyberfantasia.pcm", "r");
  if (!f_driver || !f_pcm) {
    return -1;
  }

  // make the read/writes use PCM_BUFFER size
  // because mlibc by default uses 4096, which is stupidly tiny for audio
  setvbuf(f_driver, NULL, _IONBF, PCM_BUFFER);
  setvbuf(f_pcm, NULL, _IONBF, PCM_BUFFER);

  char* buffer = malloc(PCM_BUFFER);
  if (!buffer) {
    return -1;
  }

  long pcm_pos = 0;
  while (1) {
    fseek(f_pcm, pcm_pos, SEEK_SET);
    size_t bytes_read = fread(buffer, 1, PCM_BUFFER, f_pcm);
    if (bytes_read <= 0) break;

    long total_written = 0;
    while (total_written < bytes_read) {
      long written = fwrite(buffer, 1, PCM_BUFFER, f_driver);
      total_written += written;
    }
    pcm_pos += bytes_read;
  }
  if (fclose(f_driver) != 0) {
    return -1;
  }
  if (fclose(f_pcm) != 0) {
    return -1;
  }
  free(buffer);

  return 0;
}
