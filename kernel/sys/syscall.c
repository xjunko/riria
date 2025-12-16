#include <riria/cpu/isr.h>
#include <riria/cpu/regs.h>
#include <riria/libk.h>
#include <riria/serial.h>
#include <stddef.h>
#include <stdint.h>

// impl
#define SYSCALL_WRITE 4
int syscall_write(regs_t *r) {
  int fd = r->ebx;
  char *buf = (char *)r->ecx;
  size_t len = r->edx;

  if (fd == 1) {
    for (size_t i = 0; i < len; i++) {
      if (buf[i] == '\0') break;
      serial_write(buf[i]);
    }
    return len;
  }

  return -1;
}

void syscall_handler(regs_t *r) {
  switch (r->eax) {
    case SYSCALL_WRITE:
      r->eax = syscall_write(r);
      break;
    default:
      printk("[sys] unknown syscall: %d\n", r->eax);
  }
}