.section .text

.global return_to_userspace
.type return_to_userspace, @function
return_to_userspace:
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    add $8, %esp
    iret

.global enter_userspace
.type enter_userspace, @function
enter_userspace:
    cli

    # copy parameters to ecx
    # param:
    # - entry_point (4)
    mov %esp, %ecx
    
    # select the segment, 0x23 = user data/code segment
    mov $0x23, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    # prepare
    pushl $0x23          # SS - user data
    pushl $0x2000000     # ESP - user stack top
    pushfl               # EFLAGS

    popl %eax            # Get EFLAGS

    # request ring 3
    orl $0x200, %eax     # 
    pushl %eax           # 
    pushl $0x1B          # CS (user code segment)

    # push entry point
    pushl 4(%ecx)        # EIP - entry point
    
    # here goes nothing
    iret

    # should never reach here
    popl %ecx
    ret
