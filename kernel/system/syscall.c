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

#ifdef DEBUG
  printf("[sys] syscall_write: fd=%d buf=%p len=%d\n", fd, buf, len);
#endif

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

#define SYSCALL_SEEK 8
int syscall_seek(sysregs_t* r) {
  int fd = r->rdi;
  size_t offset = (size_t)r->rsi;
  int whence = (int)r->rdx;

  int ret = vfs_sys_seek(fd, offset, whence);
  if (ret < 0) {
    ret = -1;
  }

  return ret;
}

#define SYSCALL_GET_THREAD_ID 30
int syscall_get_thread_id(sysregs_t* r) {
  UNUSED(r);
  return process_get_current()->id;
}

#define SYSCALL_MMAP 31
int syscall_mmap(sysregs_t* r) {
  r->rax = process_get_current()->user_heap_position;
  uint32_t heap_pages = ALIGN_UP(r->rdi, PAGE_SIZE) / PAGE_SIZE;
  printf("[sys] syscall_mmap: size=%lu pages=%u curr_pos=0x%lx\n", r->rdi,
         heap_pages, r->rax);

  for (uint32_t i = 0; i < heap_pages; i++) {
    uintptr_t page = (uintptr_t)pmm_allocate();
    vmm_map_page(process_get_current()->pagemap,
                 process_get_current()->user_heap_position, page,
                 PTE_PRESENT | PTE_USER | PTE_WRITABLE | PTE_NX);
    process_get_current()->user_heap_position += PAGE_SIZE;
  }
  printf("[sys] new heap position: 0x%lx\n",
         process_get_current()->user_heap_position);
  return r->rax;
}

#define SYSCALL_WRITEFSBASE 32
int syscall_write_fsbase(sysregs_t* r) {
  uint64_t fsbase = r->rdi;

  printf("[sys] syscall_write_fsbase: fsbase=0x%lx\n", fsbase);
  if (fsbase <= 0x00007FFFFFFFFFFF) {
    printf("[sys] writing fsbase to 0x%lx\n", fsbase);
    wrmsr(MSR_FS_BASE, fsbase);
    return 0;
  }

  return -1;
}

void syscall_handler(sysregs_t* r) {
#ifdef DEBUG
  printf("[sys] syscall invoked: %d\n", r->rax);
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
    case SYSCALL_SEEK:
      r->rax = syscall_seek(r);
      break;
    case SYSCALL_GET_THREAD_ID:
      r->rax = syscall_get_thread_id(r);
      break;
    case SYSCALL_MMAP:
      r->rax = syscall_mmap(r);
      break;
    case SYSCALL_WRITEFSBASE:
      r->rax = syscall_write_fsbase(r);
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