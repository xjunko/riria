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

    _text_begin = .;
    .text BLOCK(4K) : ALIGN(4K) {
        *(.text .text.*)
    } :text
    _text_end = .;

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    _rodata_begin = .;
    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata .rodata.*)
    } :rodata
    _rodata_end = .;

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    _data_begin = .;
    .data BLOCK(4K) : ALIGN(4K) {
        *(.data .data.*)
    } :data
    _data_end = .;

    _bss_begin = .;
    .bss BLOCK(4K) : ALIGN(4K) {
        *(.bss .bss.*)
        *(COMMON)
    } :data
    _bss_end = .;

    end = .;

    /DISCARD/ : {
		*(.eh_frame*)
        *(.note .note.*)
	}
}