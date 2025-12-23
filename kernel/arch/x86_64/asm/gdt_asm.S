.section .text
.align 4

.global gdt_flush
.type gdt_flush, @function

gdt_flush:
    # load the new gdt
    lgdt (%rdi)

    # reload segments
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    # reload cs, far jump
    pushq $0x08
    lea .flush(%rip), %rax
    pushq %rax
    lretq

.flush:
    ret

