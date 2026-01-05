#include <riria/cpu/irq.h>
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
  printf(DEBUG "[   irq] rax=0x%x rbx=0x%x rcx=0x%x rdx=0x%x\n", r->rax, r->rbx,
         r->rcx, r->rdx);
  printf(DEBUG "[   irq] rsi=0x%x rdi=0x%x rbp=0x%x\n", r->rsi, r->rdi, r->rbp);
  printf(DEBUG "[   irq] r8=0x%x r9=0x%x r10=0x%x r11=0x%x\n", r->r8, r->r9,
         r->r10, r->r11);
  printf(DEBUG "[   irq] r12=0x%x r13=0x%x r14=0x%x r15=0x%x\n", r->r12, r->r13,
         r->r14, r->r15);
}

// impl
#define SYSCALL_RESTART 0
int syscall_restart(sysregs_t* r) {
  UNUSED(r);
  UNREACHABLE();
  return 0;
}

#define SYSCALL_EXIT 1
int syscall_exit(sysregs_t* r) {
  int code = r->rdi;
#ifdef KDEBUG
  printf(DEBUG "[   sys] syscall_exit: code=%d\n", code);
#endif
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

#ifdef KDEBUG
  printf(DEBUG "[   sys] syscall_read: fd=%d buf=%p len=%d\n", fd, buf, len);
#endif

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

#ifdef KDEBUG
  printf(DEBUG "[   sys] syscall_write: fd=%d buf=%p len=%d\n", fd, buf, len);
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

#ifdef KDEBUG
  printf(DEBUG "[   sys] syscall_open: path=%s flags=0x%x mode=0o%x\n", path,
         flags, mode);
#endif
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

#define SYSCALL_WRITEFSBASE 29
int syscall_write_fsbase(sysregs_t* r) {
  uint64_t fsbase = r->rdi;

#ifdef KDEBUG
  printf(DEBUG "[   sys] syscall_write_fsbase: fsbase=0x%lx\n", fsbase);
#endif

  if (fsbase <= 0x00007FFFFFFFFFFF) {
    printf(DEBUG "[   sys] writing fsbase to 0x%lx\n", fsbase);
    wrmsr(MSR_FS_BASE, fsbase);
    return 0;
  }

  return -1;
}

#define SYSCALL_GET_THREAD_ID 30
int syscall_get_thread_id(sysregs_t* r) {
  UNUSED(r);
  return process_get_current()->id;
}

#define SYSCALL_MMAP 31
int syscall_mmap(sysregs_t* r) {
  // start - r->rdi
  // size - r->rsi
  // prot - r->rdx
  // flags - r->r10
  // fd - r->r8
  // offset - r->r9

  uint64_t fd = r->r8;
  uint64_t offset = r->r9;
  uint64_t flags = r->r10;
  size_t size = r->rsi;

  process_t* current = process_get_current();
  uintptr_t heap_start = current->user_heap_position;
  uint32_t heap_pages = ALIGN_UP(r->rsi, PAGE_SIZE) / PAGE_SIZE;

#ifdef KDEBUG
  printf(DEBUG "[   sys] process %d heap position: 0x%lx\n", current->id,
         current->user_heap_position);
  printf(DEBUG
         "[   sys] syscall_mmap: start=0x%lx size=0x%lx prot=0x%x flags=0x%x "
         "fd=%d "
         "offset=0x%lx\n",
         heap_start, r->rsi, (uint32_t)r->rdx, (uint32_t)r->r10, (int)r->r8,
         (uint64_t)r->r9);
  printf(DEBUG "[   sys] mmap with fd %d w/ %d pages, offset=0x%lx\n", fd,
         heap_pages, offset);
#endif

  // we handle this by two cases, one with fd and one without
  if ((int)fd != -1) {
    printf(DEBUG "[   sys] mmap with fd, using vfs_sys_mmap\n");
    uint64_t data = (uint64_t)vfs_sys_mmap(fd, &size, flags, offset);
    ASSERT(data != 0);
    for (uint32_t i = 0; i < heap_pages; i++) {
      vmm_map_page(current->pagemap, current->user_heap_position, data,
                   PTE_PRESENT | PTE_USER | PTE_WRITABLE);
      data += PAGE_SIZE;
      current->user_heap_position += PAGE_SIZE;
    }
  } else {
    for (uint32_t i = 0; i < heap_pages; i++) {
      uintptr_t page = (uintptr_t)pmm_allocate();
      ASSERT(page != 0);
      vmm_map_page(current->pagemap, current->user_heap_position, page,
                   PTE_PRESENT | PTE_USER | PTE_WRITABLE | PTE_NX);
      current->user_heap_position += PAGE_SIZE;
    }
  }

#ifdef KDEBUG
  printf(DEBUG "[   sys] new heap position: 0x%lx\n",
         current->user_heap_position);
  printf(DEBUG "[   sys] mmap returning address: 0x%lx\n", heap_start);
#endif

  return (int)heap_start;
}

#define SYSCALL_UNMAP 32
int syscall_unmap(sysregs_t* r) {
  process_t* current = process_get_current();
  uintptr_t addr = r->rdi;
  uintptr_t base = addr & ~(PAGE_SIZE - 1);
  size_t size = r->rsi;
  size_t pages = ALIGN_UP(size, PAGE_SIZE) / PAGE_SIZE;

  if (addr < process_get_current()->user_heap_start) {
    panic("WTF");
    printf(
        "[sys] syscall_unmap: invalid address 0x%lx below heap start 0x%lx\n",
        addr, process_get_current()->user_heap_start);

    return -1;
  }

  if (addr + size > current->user_heap_position) {
    panic("WTF");
    printf(
        "[sys] syscall_unmap: invalid address 0x%lx + size 0x%lx above "
        "heap position 0x%lx\n",
        addr, size, process_get_current()->user_heap_position);

    return -1;
  }

#ifdef KDEBUG
  printf(DEBUG "[   sys] syscall_unmap: addr=0x%lx size=0x%lx pages=%d\n", addr,
         size, pages);
#endif

  for (uint32_t i = 0; i < pages; i++) {
    uintptr_t virt = base + i * PAGE_SIZE;
    uintptr_t phys = vmm_virt_to_phys(current->pagemap, virt);
    ASSERT(phys != 0);

    pmm_free((void*)phys);
    vmm_unmap_page(current->pagemap, virt);
  }

#ifdef KDEBUG
  printf(DEBUG "[   sys] unmap completed\n");
#endif

  return 0;
}

// the syscall are based loosely from linux'x x86_32 syscall numbers
// https://chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md#x86-32_bit
void syscall_handler(sysregs_t* r) {
  IRQ_OFF;
#ifdef KDEBUG
  printf(DEBUG "[   sys] syscall invoked: %d\n", r->rax);
  print_sysregs(r);
#endif
  switch (r->rax) {
    case SYSCALL_RESTART:
      r->rax = syscall_restart(r);
      break;
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
    case SYSCALL_WRITEFSBASE:
      r->rax = syscall_write_fsbase(r);
      break;
    case SYSCALL_GET_THREAD_ID:
      r->rax = syscall_get_thread_id(r);
      break;
    case SYSCALL_MMAP:
      r->rax = syscall_mmap(r);
      break;
    case SYSCALL_UNMAP:
      r->rax = syscall_unmap(r);
      break;
    case 77:
      r->rax = ticks;
      break;
    default:
      r->rax = -1;
      printf(ERROR "[   sys] unknown syscall: %d\n", r->rax);
      break;
  }
#ifdef KDEBUG
  printf(DEBUG "[   sys] syscall completed, return value: %d\n", r->rax);
#endif
  IRQ_RES;
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