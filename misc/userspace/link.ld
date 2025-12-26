ENTRY(_start)

SECTIONS {
    . = 0x10000;

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}
