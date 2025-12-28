.section .text
.align 4

.global switch_to_user
.type switch_to_user, @function
switch_to_user:
    pushq $0x23    # user data segment
    pushq %rsi     # user stack pointer
    pushq $0x200   # rflags
    pushq $0x2B    # user code segment
    pushq %rdi     # entry point

    iretq
