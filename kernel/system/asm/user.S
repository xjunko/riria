
.equ USER_STACK_TOP, 0x0000000080000000

.global switch_to_user
.type switch_to_user, @function
switch_to_user:
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
    pushq $0x10000 # user instruction pointer

    iretq
