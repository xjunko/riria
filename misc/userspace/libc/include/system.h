#pragma once
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define UNUSED(x) (void)(x)

long syscall0(long n);
long syscall1(long n, long a1);
long syscall2(long n, long a1, long a2);
long syscall3(long n, long a1, long a2, long a3);