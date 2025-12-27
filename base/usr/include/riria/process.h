#pragma once
#include <riria/cpu/regs.h>
#include <riria/mem.h>
#include <riria/types.h>

#define USER_VIRT_START 0x10000  // entry are expected to be loaded here
#define STACK_SIZE 0x10000
#define USER_STACK_TOP 0x80000000
#define USER_STACK_BASE (USER_STACK_TOP - STACK_SIZE)
#define USER_VIRT_END (USER_STACK_BASE + STACK_SIZE)

typedef enum {
  PROCESS_KERNEL,
  PROCESS_USER,
} process_type_t;

typedef enum {
  PROCESS_READY,
  PROCESS_RUNNING,
  PROCESS_SLEEPING,
  PROCESS_DEAD,
} process_state_t;

typedef struct process {
  // rsp for both kernel and user mode
  // because of reasons, the assembly expects these to be in this order
  // check kernel/system/asm/syscall.S
  uint64_t krsp;  // 0
  uint64_t ursp;  // 8

  uint32_t id;
  const char* name;
  process_type_t type;
  process_state_t state;

  void* stack;
  void* stack_top;
  pagemap_t* pagemap;
} process_t;

typedef struct process_node {
  process_t* process;
  struct process_node* next;
} process_node_t;

typedef void (*process_entry_t)(void);

void process_create(process_entry_t, pagemap_t*);
void process_reap(void);
int process_schedule(regs_t* r);
void process_exit(int);

process_t* process_get_current(void);

__attribute__((noreturn)) void process_spawn_user(const uint8_t*, size_t,
                                                  uint64_t);
__attribute__((noreturn)) void process_spawn_elf(uint8_t*, size_t);

__attribute__((noreturn)) extern void process_switch(uint64_t*, uint64_t*);
__attribute__((noreturn)) extern void switch_to_user(uint64_t);