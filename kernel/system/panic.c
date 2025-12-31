#include <riria/cpu/irq.h>
#include <riria/process.h>
#include <riria/types.h>
#include <stdio.h>

__attribute__((noreturn)) void panic(const char* message) {
  IRQ_OFF;
  const bool is_user = (process_get_current()->type == PROCESS_USER);
  const char* user_or_kernel = is_user ? "USER" : "KERNEL";

  printf(ERROR "[ error] %s PANIC: %s\n", user_or_kernel, message);
  if (is_user) {
    printf(WARNING "[ error] Attempting to kill user process %u (name=%s)\n",
           process_get_current()->id, process_get_current()->name);
    process_exit(-1);  // hopefully thats all we needed to do.
  }

  while (1) asm volatile("hlt");
}