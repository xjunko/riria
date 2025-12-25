## riria-kernel-x64

currently the kernel boots up and prints some stuff, not that interesting.

<img width="1041" height="836" alt="image" src="https://github.com/user-attachments/assets/3248f3d7-e82a-4026-9376-e4046df91c97" />


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
    - we have basic non-elf exec working
    - userspace somewhat works now
- scheduling
    - stupidly basic round robin scheduler, it does the job.
- kernel shell
    - it can play music and run the test executable

### things that doesnt work:
- proper libc
- userspace shell
- exec inside userspace

### license:
ISC License
