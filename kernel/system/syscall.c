#include <riria/cpu/isr.h>
#include <riria/cpu/regs.h>
#include <riria/drivers/serial.h>
#include <riria/types.h>
#include <stdio.h>

// impl
#define SYSCALL_WRITE 4
int syscall_write(regs_t* r) {
  int fd = r->rbx;
  char* buf = (char*)r->rcx;
  size_t len = r->rdx;

  if (fd == 1) {
    for (size_t i = 0; i < len; i++) {
      if (buf[i] == '\0') break;
      serial_write(buf[i]);
    }
    return len;
  }

  return -1;
}

void syscall_handler(regs_t* r) {
  switch (r->rax) {
    case SYSCALL_WRITE:
      r->rax = syscall_write(r);
      break;
    default:
      kprintf("[sys] unknown syscall: %d\n", r->rax);
  }
}