#include <riria/cpu/irq.h>
#include <riria/process.h>
#include <unistd.h>

unsigned int sleep(unsigned int seconds) {
  process_sleep(seconds * 1000);
  return 0;
}