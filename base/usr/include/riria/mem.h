#pragma once
#include <riria/boot.h>

#define PAGE_SIZE 4096

#define VMM_HIGHER_HALF (hhdm_request.response->offset)

void pmm_install(void);
void* pmm_alloc(void);
void pmm_free(void*);