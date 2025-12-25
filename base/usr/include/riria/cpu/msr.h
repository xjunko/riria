#pragma once
#include <riria/types.h>

// https://github.com/xen-project/xen/blob/master/xen/arch/x86/include/asm/msr-index.h

#define MSR_EFER 0xC0000080 /* Extended Feature Enable Register */

#define MSR_STAR 0xC0000081         /* legacy mode SYSCALL target */
#define MSR_LSTAR 0xC0000082        /* long mode SYSCALL target */
#define MSR_CSTAR 0xC0000083        /* compat mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xC0000084 /* EFLAGS mask for syscall */

#define MSR_USER_GS_BASE 0xC0000101   /* 64bit GS base */
#define MSR_KERNEL_GS_BASE 0xC0000102 /* 64bit GS base */

uint64_t rdmsr(uint64_t);
void wrmsr(uint64_t, uint64_t);