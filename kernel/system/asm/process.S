.section .text
.align 4

.global process_switch
.type process_switch, @function
process_switch:
    push %rbp
    push %rbx
    push %r12
    push %r13
    push %r14
    push %r15

    mov %rsp, (%rdi)      # save old rsp to *prev
    mov (%rsi), %rsp      # load new rsp from *next

    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbx
    pop %rbp
    
    ret


# kernel
.extern process_kernel_entry
.type process_kernel_entry, @function

.global process_kernel_trampoline
.type process_kernel_trampoline, @function
process_kernel_trampoline:
    mov %r15, %rdi  
    call process_entry
    ud2

# user
.extern process_user_entry
.type process_user_entry, @function

.global process_user_trampoline
.type process_user_trampoline, @function
process_user_trampoline:
    mov %r15, %rdi  
    mov %r14, %rsi
    call process_user_entry
    ud2