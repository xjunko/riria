#pragma once
// it gets old having to include the same damn thing everytime, lets have it all
// in one place
#include <riria/tty.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// we dont have a proper libc yet, so this will have to do for now
#ifdef __i386__
typedef uint32_t size_t;
typedef int32_t ssize_t;
#endif

#ifdef __x86_64__
typedef uint64_t size_t;
typedef int64_t ssize_t;
#endif

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define UNUSED(x) (void)(x)

#define ASSERT(cond)                                                        \
  do {                                                                      \
    if (!(cond)) {                                                          \
      printf(ERROR "[ error] Assertion failed: %s\n", #cond);               \
      printf(ERROR "[ error] In file: %s, line: %d\n", __FILE__, __LINE__); \
      panic("Assertion failure");                                           \
    }                                                                       \
  } while (0)

#define UNREACHABLE() ASSERT(0)

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#if defined(__clang__)
#define COMPILER_NAME "Clang"
#define COMPILER_VERSION_STRING \
  STR(__clang_major__) "." STR(__clang_minor__) "." STR(__clang_patchlevel__)
#elif defined(__GNUC__)
#define COMPILER_NAME "GCC"
#define COMPILER_VERSION_STRING \
  STR(__GNUC__) "." STR(__GNUC_MINOR__) "." STR(__GNUC_PATCHLEVEL__)
#else
#define COMPILER_NAME "Unknown"
#define COMPILER_VERSION_STRING "0.0.0"
#endif

#define OS_NAME "riria"
#define OS_CODENAME "aina"
#define OS_VERSION "0.0.1a"

#define HALT()                    \
  do {                            \
    for (;;) asm volatile("hlt"); \
  } while (0)

void kprintf(const char*, ...);
__attribute__((noreturn)) void panic(const char*);