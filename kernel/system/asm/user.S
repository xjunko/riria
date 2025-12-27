.section .text
.align 4

.equ USER_STACK_TOP, 0x80000000

.global switch_to_user
.type switch_to_user, @function
switch_to_user:
    # RDI contains the entry point address
    mov %rdi, %r15  # Save entry point to r15
    
    mov $0x23, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    pushq $0x23 # user data segment
    mov $USER_STACK_TOP, %rax # user stack pointer
    pushq %rax 
    pushq $0x200   # rflags
    pushq $0x2B    # user code segment
    pushq %r15     # user instruction pointer (entry point from parameter)

    iretq
