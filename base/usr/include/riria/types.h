#pragma once
// it gets old having to include the same damn thing everytime, lets have it all
// in one place
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

#define ASSERT(cond)                                               \
  do {                                                             \
    if (!(cond)) {                                                 \
      printf("[err] Assertion failed: %s\n", #cond);               \
      printf("[err] In file: %s, line: %d\n", __FILE__, __LINE__); \
      panic("Assertion failure");                                  \
    }                                                              \
  } while (0)

#define UNREACHABLE() ASSERT(0)

void kprintf(const char*, ...);
void panic(const char*);