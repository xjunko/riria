.section .text
.align 4

.global switch_to_user
.type switch_to_user, @function
switch_to_user:
    mov %rdi, %r15 # entry point
    mov %rsi, %r14 # user stack pointer

    pushq $0x23    # user data segment
    pushq %r14     # user stack pointer
    pushq $0x200   # rflags
    pushq $0x2B    # user code segment
    pushq %r15     # entry point

    iretq
