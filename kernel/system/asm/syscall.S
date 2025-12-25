.section .text
.align 4

.extern syscall_handler
.type syscall_handler, @function

.global syscall_trap
.type syscall_trap, @function
syscall_trap:
    swapgs
    cli 

    mov %rsp, %gs:32
    mov %gs:24, %rsp

    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp

    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

    mov %rsp, %rdi
    call syscall_handler

    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    mov %rsp, %gs:24
    mov %gs:32, %rsp

    swapgs
    sysretq
