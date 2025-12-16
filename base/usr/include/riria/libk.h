#pragma once
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

void *memset(void *, int, size_t);
void *memcpy(void *restrict, const void *restrict, size_t);

int vsprintf(char *buffer, const char *fmt, va_list args);
void printk(const char *fmt, ...);