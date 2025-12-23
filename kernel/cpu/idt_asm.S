.section .text
.align 4

.global idt_flush
.type idt_flush, @function

idt_flush:
    lidt (%rdi)
    ret