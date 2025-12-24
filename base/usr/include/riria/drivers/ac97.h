#pragma once
#include <riria/types.h>

typedef struct audio_buffer {
  volatile uint8_t* data;
  size_t data_written;
  uintptr_t phys;
  int has_played;
} audio_buffer_t;

void ac97_install(void);

void ac97_set_volume(int);

int ac97_write_buffer(uint8_t*, size_t);
int ac97_write_pcm(uint8_t*, size_t);
void ac97_flush(void);

int ac97_can_write(void);
