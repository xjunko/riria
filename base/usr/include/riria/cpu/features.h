#pragma once
#include <riria/types.h>

static inline uint64_t read_cr0(void) {
  uint64_t value;
  __asm__ volatile("mov %%cr0, %0" : "=r"(value) : : "memory");
  return value;
}

static inline void write_cr0(uint64_t value) {
  __asm__ volatile("mov %0, %%cr0" : : "r"(value) : "memory");
}

static inline uint64_t read_cr4(void) {
  uint64_t value;
  __asm__ volatile("mov %%cr4, %0" : "=r"(value) : : "memory");
  return value;
}

static inline void write_cr4(uint64_t value) {
  __asm__ volatile("mov %0, %%cr4" : : "r"(value) : "memory");
}