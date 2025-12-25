.section .text
.align 4

.global tss_flush
.type tss_flush, @function

tss_flush:
    mov $0x30, %ax
    ltr %ax
    ret