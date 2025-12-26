#include <riria/cpu/isr.h>
#include <riria/cpu/msr.h>
#include <riria/cpu/regs.h>
#include <riria/drivers/serial.h>
#include <riria/fs/vfs.h>
#include <riria/process.h>
#include <riria/syscall.h>
#include <riria/types.h>
#include <stdio.h>
#include <string.h>

static void print_sysregs(sysregs_t* r) {
  printf("[irq] rax=0x%x rbx=0x%x rcx=0x%x rdx=0x%x\n", r->rax, r->rbx, r->rcx,
         r->rdx);
  printf("[irq] rsi=0x%x rdi=0x%x rbp=0x%x\n", r->rsi, r->rdi, r->rbp);
  printf("[irq] r8=0x%x r9=0x%x r10=0x%x r11=0x%x\n", r->r8, r->r9, r->r10,
         r->r11);
  printf("[irq] r12=0x%x r13=0x%x r14=0x%x r15=0x%x\n", r->r12, r->r13, r->r14,
         r->r15);
}

// impl
#define SYSCALL_EXIT 1
int syscall_exit(sysregs_t* r) {
  int code = r->rdi;
  printf("[sys] syscall_exit: code=%d\n", code);
  process_exit(code);
  r->rax = 0;
  return 0;
}

#define SYSCALL_FORK 2
int syscall_fork(sysregs_t* r) {
  UNUSED(r);
  UNREACHABLE();
  return -1;
}

#define SYSCALL_READ 3
int syscall_read(sysregs_t* r) {
  int fd = r->rdi;
  char* buf = (char*)r->rsi;
  size_t len = r->rdx;

  int ret = vfs_sys_read(fd, buf, len);
  if (ret < 0) {
    ret = -1;
  }

  return ret;
}

#define SYSCALL_WRITE 4
int syscall_write(sysregs_t* r) {
  // internally, we split the fd by two category, above the VFS_FD_OFFSET are
  // normal fd, anything below is special case.
  int fd = r->rdi;
  char* buf = (char*)r->rsi;
  size_t len = r->rdx;

  printf("[sys] syscall_write: fd=%d buf=%p len=%d\n", fd, buf, len);

  // system fds
  if (fd < VFS_FD_OFFSET) {
    switch (fd) {
      case 1: {
        for (size_t i = 0; i < len; i++) {
          char c = buf[i];
          if (c == '\0') break;
          kprintf("%c", c);
        }

        return len;
      }
      default:
        return -1;
    }
  }

  // user fds
  if (fd >= VFS_FD_OFFSET) {
    int ret = vfs_sys_write(fd, buf, len);
    if (ret < 0) {
      ret = -1;
    }

    return ret;
  }

  // not found
  return -1;
}

#define SYSCALL_OPEN 5
int syscall_open(sysregs_t* r) {
  char* path = (char*)r->rdi;
  int flags = (int)r->rsi;
  int mode = (int)r->rdx;

  int ret = vfs_sys_open(path, flags, mode);
  if (ret < 0) {
    ret = -1;
  }

  return ret;
}

#define SYSCALL_CLOSE 6
int syscall_close(sysregs_t* r) {
  int fd = r->rdi;

  int ret = vfs_sys_close(fd);
  if (ret < 0) {
    ret = -1;
  }

  return ret;
}

void syscall_handler(sysregs_t* r) {
  printf("[sys] syscall invoked: %d\n", r->rax);
#ifdef DEBUG
  print_sysregs(r);
#endif

  switch (r->rax) {
    case SYSCALL_EXIT:
      r->rax = syscall_exit(r);
      break;
    case SYSCALL_FORK:
      r->rax = syscall_fork(r);
      break;
    case SYSCALL_READ:
      r->rax = syscall_read(r);
      break;
    case SYSCALL_WRITE:
      r->rax = syscall_write(r);
      break;
    case SYSCALL_OPEN:
      r->rax = syscall_open(r);
      break;
    case SYSCALL_CLOSE:
      r->rax = syscall_close(r);
      break;
    default:
      r->rax = -1;
      printf("[sys] unknown syscall: %d\n", r->rax);
      break;
  }
}

//
void syscall_install(void) {
  uint64_t efer = rdmsr(MSR_EFER);
  efer |= (1 << 0);
  wrmsr(MSR_EFER, efer);

  uint64_t star = ((uint64_t)(0x18 | 3) << 48) | ((uint64_t)0x08 << 32);
  wrmsr(MSR_STAR, star);

  wrmsr(MSR_LSTAR, (uint64_t)syscall_trap);
  wrmsr(MSR_SYSCALL_MASK, ~0x2);
}