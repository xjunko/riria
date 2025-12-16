.section .text

.global _start
.type _start, @function

.extern kmain

_start:
    mov $kernel_stack_end, %esp

    pushl %ebx
    pushl %eax
    call kmain

    cli
hang:
    hlt
    jmp hang

kernel_stack:
    .space 2 * 1024 * 1024
kernel_stack_end:
