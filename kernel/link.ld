ENTRY(kmain)
OUTPUT_FORMAT(elf64-x86-64)

PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
}


SECTIONS {
    . = 0xffffffff80000000;
    
    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .text BLOCK(4K) : ALIGN(4K) {
        *(.text .text.*)
    } :text

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata .rodata.*)
    } :rodata

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data BLOCK(4K) : ALIGN(4K) {
        *(.data .data.*)
    } :data

    .bss BLOCK(4K) : ALIGN(4K) {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    end = .;

    /DISCARD/ : {
		*(.eh_frame*)
        *(.note .note.*)
	}
}