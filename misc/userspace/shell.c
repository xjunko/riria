#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PCM_BUFFER 128000

int play_pcm(const char* args) {
  const char* path = args;
  if (!path) printf("no path specified.");

  char path_to_mus[256] = "/init/";
  strcat(path_to_mus, path);

  printf("playing %s\n", path_to_mus);

  FILE* f_driver = fopen("/dev/audio", "w");
  FILE* f_pcm = fopen(path_to_mus, "r");
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
    size_t bytes_read = fread(buffer, 1, PCM_BUFFER, f_pcm);
    if (bytes_read <= 0) break;

    long total_written = 0;
    while (total_written < bytes_read) {
      long written = fwrite(buffer, 1, PCM_BUFFER, f_driver);
      total_written += written;
    }

    pcm_pos += bytes_read;
    printf("played %ld bytes\r", pcm_pos);
  }
  free(buffer);
  if (fclose(f_driver) != 0) {
    return -1;
  }
  if (fclose(f_pcm) != 0) {
    return -1;
  }

  return 0;
}

int cat(const char* args) {
  const char* path = args;
  if (!path) {
    printf("no path specified.");
    return -1;
  }

  char path_to_file[256] = "/init/";
  strcat(path_to_file, path);

  FILE* f = fopen(path_to_file, "r");
  if (!f) {
    printf("failed to open %s\n", path_to_file);
    return -1;
  }

  char buffer[512];
  size_t bytes_read;

  while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
    fwrite(buffer, 1, bytes_read, stdout);
  }
  printf("\n");

  fclose(f);
  return 0;
}

void exec(const char* arg) {
  // arg0 - cmd
  // arg1... - args
  const char* cmd = strtok((char*)arg, " ");
  const char* args = strtok(NULL, "");

  if (strcmp(cmd, "cat") == 0) {
    cat(args);
  }

  if (strcmp(cmd, "play") == 0) {
    play_pcm(args);
  }
}

char getch(void) {
  FILE* f_kb = fopen("/dev/keyboard", "r");
  if (!f_kb) return '\0';
  setvbuf(f_kb, NULL, _IONBF, 0);

  char key[1] = {'\0'};
  while (1) {
    int ret = fread(key, 1, 1, f_kb);
    if (key[0] != '\0') {
      fclose(f_kb);
      return key[0];
    }
  }
}

size_t read_line(char* buf, size_t len) {
  size_t pos = 0;

  while (1) {
    char c = getch();

    switch (c) {
      case '\n':
        printf("\n");
        return pos;
        break;
      case '\b':
        if (pos > 0) {
          pos -= 1;
          printf("\b \b");
        }
        break;
      case 32 ... 126:
        if (pos < len - 1) {
          buf[pos] = c;
          pos++;
          printf("%c", c);
        }
        break;
      default:
        break;
    }
  }
}

#define SHELL_BUFFER 128
int main(void) {
  char buffer[SHELL_BUFFER] = {'\0'};

  // dont need it to be buffered
  setvbuf(stdout, NULL, _IONBF, 0);

  while (1) {
    printf("> ");

    size_t len = read_line(buffer, SHELL_BUFFER);
    memset(buffer + len, 0, SHELL_BUFFER - len);
    if (len == 0) continue;

    exec(buffer);
  }

  return 0;
}