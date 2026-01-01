#include <riria/cpu/irq.h>
#include <riria/process.h>
#include <unistd.h>

unsigned int sleep(unsigned int seconds) {
  uint32_t millisecond = seconds * 1000;
  uint32_t start_ticks = ticks;

  while (ticks - start_ticks < millisecond) {
    process_yield();
  }

  return 0;
}