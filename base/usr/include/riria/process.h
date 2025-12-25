#pragma once
#include <riria/cpu/regs.h>
#include <riria/mem.h>
#include <riria/types.h>

typedef struct process {
  uint32_t id;

  void* stack;
  void* stack_top;
  uint64_t krsp;
  uint64_t ursp;
  pagemap_t* pagemap;

  bool is_user;
} process_t;

typedef struct process_node {
  process_t* process;
  struct process_node* next;
} process_node_t;

#define STACK_SIZE 0x10000

typedef void (*process_entry_t)(void);

void process_create(process_entry_t, pagemap_t*);
int process_schedule(regs_t* r);
process_t* process_get_current(void);

extern void process_switch(uint64_t*, uint64_t*);