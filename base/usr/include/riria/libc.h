#pragma once
#include <riria/types.h>

void* malloc(size_t);

void* memset(void*, int, size_t);
void* memcpy(void* restrict, const void* restrict, size_t);
void* memmove(void*, const void*, size_t);
int memcmp(const void*, const void*, size_t);

size_t strlen(const char*);

int printf(const char*, ...);
int vsprintf(char*, const char*, va_list);
int snprintf(char*, size_t, const char*, ...);

void kprintf(const char*, ...);