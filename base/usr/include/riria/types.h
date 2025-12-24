#pragma once
// it gets old having to include the same damn thing everytime, lets have it all
// in one place
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// we dont have a proper libc yet, so this will have to do for now
#ifdef __i386__
typedef uint32_t size_t;
typedef int32_t ssize_t;
#endif

#ifdef __x86_64__
typedef uint64_t size_t;
typedef int64_t ssize_t;
#endif

#define UNUSED(x) (void)(x)

void kprintf(const char*, ...);
void panic(const char*);