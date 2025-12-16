ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS {
    . = 1M;
    phys = .;

    .text BLOCK(4K) : ALIGN(4K) {
        code = .;
        *(.multiboot2)
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K) {
        data = .;
        *(.data)
        *(.symbols)
    }

    .bss BLOCK(4K) : ALIGN(4K) {
        bss = .;
        *(COMMON)
        *(.bss)
        *(.stack)
    }

    end = .;

    /DISCARD/ : {
		*(.comment)
		*(.eh_frame)
		*(.note.gnu.build-id)
	}
}