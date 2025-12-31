#pragma once
#include <riria/types.h>

typedef volatile struct spin_lock {
  volatile int latch;
  const char* func;
} spin_lock_t;

static inline void _spin_lock(spin_lock_t* lock, const char* func,
                              const char* file, int line) {
  while (__sync_lock_test_and_set(&lock->latch, 1)) {
    printf(WARNING "[  lock] spinlock busy at %s:%d, held by %s\n", file, line,
           lock->func ? lock->func : "unknown");
    asm volatile("pause");
  }
  lock->func = func;
#ifdef KDEBUG
  printf(INFO "[  lock] spinlock acquired at %s:%d\n", file, line);
#else
  UNUSED(file);
  UNUSED(line);
#endif
}

static inline void _spin_unlock(spin_lock_t* lock, const char* file, int line) {
  lock->func = NULL;
#ifdef KDEBUG
  printf(INFO "[  lock] spinlock released at %s:%d\n", file, line);
#else
  UNUSED(file);
  UNUSED(line);
#endif
  __sync_lock_release(&lock->latch);
}

#define spin_lock(lock) _spin_lock(lock, __func__, __FILE__, __LINE__)
#define spin_unlock(lock) _spin_unlock(lock, __FILE__, __LINE__)