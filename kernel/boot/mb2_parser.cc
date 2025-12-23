#include <riria/boot.h>

void mb2_parse(uint32_t mb_info, uint32_t typ, multiboot_callback cb) {
  struct multiboot_tag *tag;

  uint32_t size = *(uint32_t *)mb_info;
  uint32_t end = mb_info + size;

  for (tag = (struct multiboot_tag *)(mb_info + 8); (uint32_t)tag < end;
       tag = (struct multiboot_tag *)(((uint32_t)tag + tag->size + 7) & ~7)) {
    if (tag->type == typ) {
      cb(tag);
    }
  }
}