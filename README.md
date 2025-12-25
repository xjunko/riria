## riria-kernel-x64

<img width="1074" height="867" alt="image" src="https://github.com/user-attachments/assets/5a615f54-d255-4860-8d31-07c100d3969d" />

### what this is
this is a very poor attempt at making an operating system, you should take everything that i did here as a what not to do.

### what this is not
- an original project, most of the kernel wouldn't even work without me referencing other osdev projects.

### things that work:
- gdt/idt/irq/isr 
    - tss is hacky, but its fine for now.
- syscall 
    - currently only write(4) which writes to stdout to get the ball rolling
- pmm + vmm + paging
    - super basic, but it works now.
- malloc and family
    - we have kmalloc and kfree, good enough.
- vfs
    - devfs, tarfs
    - /dev/, /init/
- PCI
    - ac97 audio driver which somewhat works
- elf exec
    - ELF works, though, it's a bit janky atm with the userspace linker setup.
    - userspace is okayish now
- scheduling
    - super basic singly linked list round robin scheduling, good enough.
- kernel shell
    - it can play music and run the test executable


### things that doesnt work:
- proper libc
- userspace shell
- exec inside userspace

### license:
ISC License
