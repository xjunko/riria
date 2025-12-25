#pragma once
#include <riria/drivers/ps2.kb.set.h>
#include <riria/types.h>

#define KB_BUFFER_SIZE 1024

#define KB_PUSH(c)                               \
  do {                                           \
    input_buffer[input_buffer_index++] = c;      \
    ASSERT(input_buffer_index < KB_BUFFER_SIZE); \
  } while (0)

#define KB_POP() \
  (input_buffer_index > 0 ? input_buffer[--input_buffer_index] : 0)

#define KB_BUFFER_INDEX() (input_buffer_index)

#define KB_DRIVER_USER()                   \
  extern volatile char input_buffer[1024]; \
  extern volatile size_t input_buffer_index;

int ps2_keyboard_install(void);
int ps2_keyboard_uninstall(void);

int ps2_mouse_install(void);
int ps2_mouse_uninstall(void);