#include <riria/boot.h>
#include <riria/libc.h>
#include <riria/types.h>

// clang-format off
__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(4);
// clang-format on

void boot_verify(void) {
  printk("[sys] verifying bootloader... \n");

  if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
    while (1) asm volatile("hlt");
  }

  printk("[sys] bootloader verified.\n");
}
