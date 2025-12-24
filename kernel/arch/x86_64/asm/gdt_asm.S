.section .text
.align 4

.global gdt_flush
.type gdt_flush, @function

gdt_flush:
    lgdt (%rdi)

    pushq $0x08
    lea reload_cs(%rip), %rax
    pushq %rax
    retfq

reload_cs:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ret

.flush:
    ret

