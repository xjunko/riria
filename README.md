## riria-kernel

currently the kernel boots up and prints some stuff, not that interesting.

### what this is
this is a very poor attempt at making an operating system, you should take everything that i did here as a what not to do.

### what this is not
- an original project, most of the kernel wouldn't even work without me referencing other osdev projects, so all [[credits]](#credits) to them.

### things that work:
- gdt/idt/irq/isr 
    - gdt is fucky but it does the job for now
- syscall 
    - currently only write(4) which writes to stdout to get the ball rolling
- elf execution 
    - stupidly basic (and hardcoded) but it works

### things that doesnt work:
- pmm
- vmm + paging
- malloc and family

### credits:
i would like to credit to these projects for making this possible in the first place
- https://github.com/klange/toaru-nih
    - most of internal are based off toaru-nih

### license:
ISC License