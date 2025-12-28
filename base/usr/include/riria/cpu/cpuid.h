#pragma once
#include <riria/types.h>

typedef struct cpuid_info {
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
} cpuid_info_t;

cpuid_info_t cpuid(uint32_t, uint32_t);