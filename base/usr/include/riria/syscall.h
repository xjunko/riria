#pragma once
#include <riria/cpu/regs.h>
#include <riria/types.h>

typedef struct sysregs {
  uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
  uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
} sysregs_t;

extern void syscall_trap(void);
void syscall_handler(sysregs_t*);
void syscall_install(void);
