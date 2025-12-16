ARCH      ?= i386
OS        ?= elf
TOOLCHAIN ?= ~/Projects/Tooling/Cross/$(ARCH)/gcc-$(ARCH)-$(OS)/bin

CC    = $(TOOLCHAIN)/$(ARCH)-$(OS)-gcc
CFLAGS = -ffreestanding -O0 -g -Wall -Wextra -Werror -Wwrite-strings -Wno-error=unused-parameter \
         -fno-pic -pedantic -I../base/usr/include

TMP_OBJ ?= /tmp/riria-obj

LIB_SRC = $(wildcard *.c)
LIB_OBJS = $(patsubst %.c,$(TMP_OBJ)/lib/%.o,$(LIB_SRC))

all: $(LIB_OBJS)

$(TMP_OBJ)/lib/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -nostdlib -c $< -o $@

objs: $(LIB_OBJS)

clean:
	rm -rf $(TMP_OBJ)/lib

.PHONY: all clean objs