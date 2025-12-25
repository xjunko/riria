.section .text
.align 4

.global process_switch
.type process_switch, @function

process_switch:
    pushfq
    push %rbx
    push %rbp
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
    pop %rbp
    pop %rbx
    popfq
    
    ret