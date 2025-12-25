ARCH      = x86_64
OS        = elf
VERSION   = 15.2.0
TOOLCHAIN = ~/Projects/Tooling/Cross/$(ARCH)/gcc-$(ARCH)-$(OS)_$(VERSION)/bin

CC    = $(TOOLCHAIN)/$(ARCH)-$(OS)-gcc
AS    = $(TOOLCHAIN)/$(ARCH)-$(OS)-as
LD    = $(TOOLCHAIN)/$(ARCH)-$(OS)-gcc
STRIP = $(TOOLCHAIN)/$(ARCH)-$(OS)-strip

EMU       = qemu-system-x86_64
EMU_ARGS  = -smp 1 -m 128M -vga virtio
EMU_ARGS += -serial stdio -no-reboot -no-shutdown \
            -audio driver=sdl,model=ac97,id=0 -enable-kvm
TMP_OBJ = /tmp/riria-obj
TMP_ISO = /tmp/riria-iso
TMP_FINAL = /tmp/riria-final

BASE = ./base

# notes:
# on clang, -fstack-protector-all instantly crashes the kernel

# compiler setup
override KERNEL_CFLAGS  = -Wall -Wextra -Werror -ffreestanding -O0
override KERNEL_CFLAGS += -fstack-protector-all -fstack-check -fsanitize=undefined \
						  -fno-lto                                                 \
                          -fno-PIC -ffunction-sections -m64 -march=x86-64          \
						  -mabi=sysv -mno-80387 -mno-mmx -mno-sse -mno-sse2        \
						  -mno-red-zone -mcmodel=kernel                            \
						  -Wno-error=unused-function -Wno-error=unused-variable
						  
override KERNEL_LDFLAGS = -nostdlib -static -z max-page-size=0x1000 

# kernel sources
KERNEL_OBJS  = $(patsubst kernel/%.c,$(TMP_OBJ)/%.o,$(wildcard kernel/*.c))
KERNEL_OBJS += $(patsubst kernel/%.c,$(TMP_OBJ)/%.o,$(wildcard kernel/*/*.c))
KERNEL_OBJS += $(patsubst kernel/%.c,$(TMP_OBJ)/%.o,$(wildcard kernel/*/*/*.c))
KERNEL_OBJS += $(patsubst kernel/%.c,$(TMP_OBJ)/%.o,$(wildcard kernel/*/*/*/*.c))
KERNEL_OBJS += $(patsubst kernel/%.c,$(TMP_OBJ)/%.o,$(wildcard kernel/*/*/*/*/*.c))

# assembly sources
# stairway to assembly heaven
KERNEL_ASMOBJS  = $(patsubst kernel/%.S,$(TMP_OBJ)/%.o,$(wildcard kernel/*.S))
KERNEL_ASMOBJS += $(patsubst kernel/%.S,$(TMP_OBJ)/%.o,$(wildcard kernel/*/*.S))
KERNEL_ASMOBJS += $(patsubst kernel/%.S,$(TMP_OBJ)/%.o,$(wildcard kernel/*/*/*.S))
KERNEL_ASMOBJS += $(patsubst kernel/%.S,$(TMP_OBJ)/%.o,$(wildcard kernel/*/*/*/*.S))
KERNEL_ASMOBJS += $(patsubst kernel/%.S,$(TMP_OBJ)/%.o,$(wildcard kernel/*/*/*/*/*.S))

# here goes nothing
.PHONY: all clean run
all: image.iso

$(TMP_OBJ):
	mkdir -p $@

$(TMP_OBJ)/%.o: kernel/%.c
	mkdir -p $(dir $@)
	$(CC) $(KERNEL_CFLAGS) -g -I$(BASE)/usr/include -Ikernel/libc/include -o $@ -c $<

$(TMP_OBJ)/%.o: kernel/%.S
	mkdir -p $(dir $@)
	$(AS) -o $@ $<

OBJS = $(KERNEL_OBJS) $(KERNEL_ASMOBJS)

$(BASE)/boot/kernel.bin: kernel/link.ld $(OBJS)
	$(LD) $(KERNEL_LDFLAGS) -T kernel/link.ld $(OBJS) -o $@

# limine stuff
$(BASE)/boot/limine/repo/limine-bios.sys:
	mkdir -p $(dir $@)
	git clone https://codeberg.org/Limine/Limine.git --branch=v10.x-binary --depth=1 $(dir $@)
	make -C $(dir $@)

# building the final image
$(TMP_FINAL)/image.iso: $(TMP_OBJ) $(BASE)/boot/kernel.bin $(BASE)/boot/limine/repo/limine-bios.sys
	mkdir -p $(TMP_ISO)/
	mkdir -p $(TMP_ISO)/boot/
	mkdir -p $(TMP_ISO)/boot/limine/
	mkdir -p $(TMP_ISO)/EFI/BOOT
	
	# copy initramfs
	cp $(BASE)/boot/initramfs.tar $(TMP_ISO)/boot/

	# copy kernel
	cp $(BASE)/boot/kernel.bin $(TMP_ISO)/boot/

	# copy limine cfg
	cp $(BASE)/boot/limine/limine.conf $(TMP_ISO)/boot/limine/

	# other limine stuff
	cp $(BASE)/boot/limine/repo/limine-bios.sys $(BASE)/boot/limine/repo/limine-bios-cd.bin \
	   $(BASE)/boot/limine/repo/limine-uefi-cd.bin \
	   $(TMP_ISO)/boot/limine/

	# efi boot tree
	cp $(BASE)/boot/limine/repo/BOOTX64.EFI $(BASE)/boot/limine/repo/BOOTIA32.EFI \
	   $(TMP_ISO)/EFI/BOOT/

	# generate final iso
	mkdir -p $(TMP_FINAL)
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
        -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        $(TMP_ISO) -o $(TMP_FINAL)/image.iso
		
run: $(TMP_FINAL)/image.iso
	$(EMU) $(EMU_ARGS) -cdrom $<

clean:
	rm -rf $(TMP_ISO)
	rm -rf $(TMP_OBJ)
	rm -rf $(TMP_FINAL)/image.iso