# riria-kernel-x64

<img width="690" height="579" alt="image" src="https://github.com/user-attachments/assets/f9f8e1a3-c1a5-4a2d-b079-d780a4b1a2f9" />

## what this is
this is a very poor attempt at making an operating system, you should take everything that i did here as a what not to do.

## what this is not
- an original project, most of the kernel wouldn't even work without me referencing other osdev projects.

## things that work:
### kernel
- gdt/idt/irq/isr
    - GDT INIT... OK! /j
    - as far as i can tell, these part of the CPU code is more or less safe now.  
- syscall 
    - exit, open, close, read, write, mmap, unmap, get_thread_id, write_fs_base
    - apparently this is enough to get a DOOM port running, so there's that. 
- pmm + vmm + paging
    - pmm, vmm and the paging are mostly based off [Arikoto's](https://codeberg.org/NerdNextDoor/arikoto), which are then modified to work with this kernel
- malloc (heap)
    - we have kmalloc, kmalloc_phys and kfree
- vfs
    - devfs, tarfs
    - open, close, read, write, seek, mmap, exists 
- PCI
    - ac97 audio driver which somewhat works
- scheduling
    - super basic singly linked list round robin scheduling, good enough.

### userspace
- program execution
    - programs now run (somewhat) properly in userspace now :D
- proper libc
    - riria's userspace uses mlibc as it's libc, pretty cool!
- tooling
    - binutils and gcc has been ported, we use these now to compile userspace applications.

## things that doesnt work:
### kernel
- filesystem
    - ext2 would be nice to have
    - pipe would be nice to have 
- multithreading
    - there is no support for multi-core
- syscalls
    - lots of syscalls are still not implemented, but basic stuff works mostly fine.
- scheduling/process
    - it would be nice to have a multiple list for waiting/ready/reap processes.
    - a lot of the code written was hacked in, i still dont like it.
- stability
    - ~~there is some weird bug with userspace shell's `play` command, it detects stack smashing a few seconds in, dunno what it was.~~ fixed (i think), apparently my IRQ_OFF/RES/ON implementation was wrong and so theres race condition everywhere, userspace seems to be a lot more stable now :D

### userspace
- programs
    - it would be nice to have more stuff ported in to the userspace, maybe one day we will have a desktop enviroment :D (a wm would be cool)
- process
    - forking
 
... and thousand other stuff that i don't remember and know yet.

### license:
ISC License
