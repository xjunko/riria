#pragma once
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define UNUSED(x) (void)(x)

static long syscall0(long n) {
  long ret;
  asm volatile("syscall" : "=a"(ret) : "a"(n) : "rcx", "r11", "memory");
  return ret;
}

static long syscall1(long n, long a1) {
  long ret;
  asm volatile("syscall"
               : "=a"(ret)
               : "a"(n), "D"(a1)
               : "rcx", "r11", "memory");
  return ret;
}

static long syscall2(long n, long a1, long a2) {
  long ret;
  asm volatile("syscall"
               : "=a"(ret)
               : "a"(n), "D"(a1), "S"(a2)
               : "rcx", "r11", "memory");
  return ret;
}

static long syscall3(long n, long a1, long a2, long a3) {
  long ret;
  asm volatile("syscall"
               : "=a"(ret)
               : "a"(n), "D"(a1), "S"(a2), "d"(a3)
               : "rcx", "r11", "memory");
  return ret;
}
