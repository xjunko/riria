ARCH      = x86_64
OS        = riria
TOOLCHAIN = ~/Projects/OSDev/Porting/toolchain/usr/bin

CC    = $(TOOLCHAIN)/$(ARCH)-$(OS)-gcc
AS    = $(TOOLCHAIN)/$(ARCH)-$(OS)-as
LD    = $(TOOLCHAIN)/$(ARCH)-$(OS)-gcc
STRIP = $(TOOLCHAIN)/$(ARCH)-$(OS)-strip

MAKE = make

ifeq (, $(shell command -v $(CC) 2>/dev/null))
$(error "x86_64-riria-gcc is required but not found, please install and change the the TOOLCHAIN variable in the Makefile accordingly")
endif

OUT_DIR := ../initramfs/bin

SRCS := $(shell find . -name '*.c')
BINS := $(addprefix $(OUT_DIR)/,$(notdir $(SRCS:.c=)))

VPATH := $(sort $(dir $(SRCS)))

.PHONY: all clean
all: $(BINS)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(OUT_DIR)/%: %.c | $(OUT_DIR)
	$(CC) $< -o $@

clean:
	rm -rf $(OUT_DIR)
