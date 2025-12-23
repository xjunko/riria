#pragma once
#include <riria/types.h>

void* malloc(size_t);

void* memset(void*, int, size_t);
void* memcpy(void* restrict, const void* restrict, size_t);
void* memmove(void*, const void*, size_t);
int memcmp(const void*, const void*, size_t);

int vsprintf(char* buffer, const char* fmt, va_list args);
void printk(const char* fmt, ...);