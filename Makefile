ARCH      = i386
OS        = elf
TOOLCHAIN = ~/Projects/Tooling/Cross/$(ARCH)/gcc-$(ARCH)-$(OS)/bin

CC    = $(TOOLCHAIN)/$(ARCH)-$(OS)-gcc
AS    = $(TOOLCHAIN)/$(ARCH)-$(OS)-as
LD    = $(TOOLCHAIN)/$(ARCH)-$(OS)-gcc
STRIP = $(TOOLCHAIN)/$(ARCH)-$(OS)-strip

EMU       = qemu-system-i386
EMU_ARGS  = -cpu pentium3 -smp 1 -m 256M -vga virtio 
EMU_ARGS += -serial stdio -no-reboot -no-shutdown

TMP_OBJ = /tmp/riria-obj
TMP_ISO = /tmp/riria-iso
TMP_FINAL = /tmp/riria-final

BASE = ./base

# compiler setup
override KERNEL_CFLAGS  = -ffreestanding -O0 -g -static -nostdlib
override KERNEL_CFLAGS += -Wall -Wextra -Werror -Wwrite-strings \
						  -Wno-error=unused-parameter -Wno-error=unused-variable -Wno-error=unused-function
override KERNEL_CFLAGS += -fno-pic -fno-pie

# kernel sources
KERNEL_OBJS  = $(patsubst kernel/%.c,$(TMP_OBJ)/%.o,$(wildcard kernel/*.c))
KERNEL_OBJS += $(patsubst kernel/%.c,$(TMP_OBJ)/%.o,$(wildcard kernel/*/*.c))
KERNEL_OBJS += $(patsubst kernel/%.c,$(TMP_OBJ)/%.o,$(wildcard kernel/*/*/*.c))

# assembly sources
KERNEL_ASMOBJS  = $(patsubst kernel/%.S,$(TMP_OBJ)/%.o,$(wildcard kernel/*.S))
KERNEL_ASMOBJS += $(patsubst kernel/%.S,$(TMP_OBJ)/%.o,$(wildcard kernel/*/*.S))
KERNEL_ASMOBJS += $(patsubst kernel/%.S,$(TMP_OBJ)/%.o,$(wildcard kernel/*/*/*.S))

# here goes nothing
.PHONY: all clean run
all: image.iso

$(TMP_OBJ):
	mkdir -p $@

$(TMP_OBJ)/%.o: kernel/%.c
	mkdir -p $(dir $@)
	$(CC) $(KERNEL_CFLAGS) -g -I$(BASE)/usr/include -o $@ -c $<

$(TMP_OBJ)/%.o: kernel/%.S
	mkdir -p $(dir $@)
	$(AS) -o $@ $<

$(TMP_OBJ)/lib/%.o: lib/%.c
	$(MAKE) -C lib TMP_OBJ=$(TMP_OBJ)

LIB_OBJS := $(patsubst lib/%.c,$(TMP_OBJ)/lib/%.o,$(wildcard lib/*.c))
OBJS = $(KERNEL_OBJS) $(KERNEL_ASMOBJS) $(LIB_OBJS)

$(BASE)/boot/kernel.bin: kernel/link.ld $(OBJS)
	$(CC) -T $< $(KERNEL_CFLAGS) -static -o $@ ${OBJS} -lgcc

$(TMP_FINAL)/image.iso: $(TMP_OBJ) $(BASE)/boot/kernel.bin
	mkdir -p $(TMP_ISO)/
	mkdir -p $(TMP_ISO)/boot/
	mkdir -p $(TMP_ISO)/boot/grub/

	# copy kernel
	cp $(BASE)/boot/kernel.bin $(TMP_ISO)/boot/

	# copy grub cfg
	cp $(BASE)/boot/grub/grub.cfg $(TMP_ISO)/boot/grub/

	# generate final iso
	mkdir -p $(TMP_FINAL)
	grub-mkrescue -o $@ $(TMP_ISO)

	# clean
	rm -rf $(TMP_ISO)
	rm -rf $(TMP_OBJ)

run: $(TMP_FINAL)/image.iso
	$(EMU) $(EMU_ARGS) -cdrom $<

clean:
	rm -rf $(TMP_ISO)
	rm -rf $(TMP_OBJ)
	rm -rf $(TMP_FINAL)/image.iso