## riria-kernel-x64

currently the kernel boots up and prints some stuff, not that interesting.

### what this is
this is a very poor attempt at making an operating system, you should take everything that i did here as a what not to do.

### what this is not
- an original project, most of the kernel wouldn't even work without me referencing other osdev projects.

### things that work:
- gdt/idt/irq/isr 
    - tss is hacky, but its fine for now.
- syscall 
    - currently only write(4) which writes to stdout to get the ball rolling

### things that doesnt work:
- pmm
- vmm + paging
- malloc and family
- elf exec
- userland
- scheduling

### license:
ISC License