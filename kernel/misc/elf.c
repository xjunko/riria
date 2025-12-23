#include <riria/elf.h>
#include <riria/libk.h>
#include <riria/types.h>

// Load ELF into memory
void load_elf(uint8_t* elf) {
  // FIXME: 64bit conversion
  // Elf32_Ehdr *eh = (Elf32_Ehdr *)elf;
  // Elf32_Phdr *ph = (Elf32_Phdr *)(elf + eh->e_phoff);

  // for (int i = 0; i < eh->e_phnum; i++) {
  //   if (ph[i].p_type != PT_LOAD) continue;

  //   memcpy((void *)ph[i].p_vaddr, elf + ph[i].p_offset, ph[i].p_filesz);
  //   memset((void *)(ph[i].p_vaddr + ph[i].p_filesz), 0,
  //          ph[i].p_memsz - ph[i].p_filesz);
  // }

  // printk("[elf] jumping to user entry at 0x%x\n", eh->e_entry);
  // printk("[elf] user stack at 0x%x\n", USER_STACK);
  // enter_userspace((uint32_t)eh->e_entry);
}
