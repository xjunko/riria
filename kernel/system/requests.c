#include <riria/boot.h>
#include <riria/types.h>

// clang-format off
__attribute__((used, section(".limine_requests")))
volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_executable_address_request executable_address_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
    .revision = 0,
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
};

__attribute__((used, section(".limine_requests")))
 volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};
// clang-format on