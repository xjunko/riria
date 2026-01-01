#pragma once
#include <riria/types.h>

int printf(const char*, ...);
int vsprintf(char*, const char*, va_list);
int snprintf(char*, size_t, const char*, ...);
int sprintf(char*, const char*, ...);