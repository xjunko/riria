.set MB2_HEADER_MAGIC,      0xE85250D6
.set MB2_ARCHITECTURE,      0 
.set MB2_HEADER_LENGTH,     MB2_HEADER_END - MB2_HEADER
.set MB2_HEADER_CHECKSUM,   -(MB2_HEADER_MAGIC + MB2_ARCHITECTURE + MB2_HEADER_LENGTH)

.section .multiboot2
    .align 8
MB2_HEADER:
    .long MB2_HEADER_MAGIC
    .long MB2_ARCHITECTURE
    .long MB2_HEADER_LENGTH
    .long MB2_HEADER_CHECKSUM

    # requests
    .short 1      // tag - info req
    .short 0      // reserved
    .long  8 + 4  // size of tag
    .long  6      // type: memory map
    .align 8

    .short 5      // tag - framebuffer
    .short 0      // reserved
    .long  20     // size of tag
    .long  1280   // target width
    .long  720    // target height
    .long  32     // target bpp
    .align 8

    .short 0      // tag - end
    .short 0      // reserved
    .long  8      // size of tag
    .align 8 
MB2_HEADER_END: