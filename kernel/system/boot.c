#include <riria/boot.h>
#include <riria/types.h>
#include <stdio.h>

// clang-format off
__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(4);
// clang-format on

void boot_verify(void) {
  printf(INFO "[   sys] %s-%s compiled with %s %s\n", OS_NAME, OS_CODENAME,
         COMPILER_NAME, COMPILER_VERSION_STRING);
  printf(DEBUG "[   sys] verifying bootloader... \n");

  if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
    panic("unsupported limine base revision!");
  }

  if (memmap_request.response == NULL) {
    panic("no memory map response!");
  }

  if (hhdm_request.response == NULL) {
    panic("no HHDM response!");
  }

  if (executable_address_request.response == NULL) {
    panic("no executable address response!");
  }

  if (module_request.response == NULL) {
    panic("no module response!");
  }

  if (module_request.response->module_count <= 0) {
    printf(WARNING "[   sys] no modules provided, oh well.\n");
  }

  printf(INFO "[   sys] bootloader verified.\n");
}
