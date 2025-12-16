ARCH      ?= i386
OS        ?= elf
TOOLCHAIN ?= ~/Projects/Tooling/Cross/$(ARCH)/gcc-$(ARCH)-$(OS)/bin

CC    = $(TOOLCHAIN)/$(ARCH)-$(OS)-gcc
AR    = $(TOOLCHAIN)/$(ARCH)-$(OS)-ar
CFLAGS = -ffreestanding -O0 -g -Wall -Wextra -Werror -Wwrite-strings -Wno-error=unused-parameter \
         -fno-pic -pedantic -I../base/usr/include

TMP_OBJ ?= /tmp/riria-obj
LIB_NAME ?= libriria.a

LIB_SRC = $(shell find . -name '*.c')
LIB_OBJS = $(patsubst ./%, $(TMP_OBJ)/lib/%, $(LIB_SRC:.c=.o))

all: $(LIB_NAME)

$(TMP_OBJ)/lib/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -nostdlib -c $< -o $@

$(LIB_NAME): $(LIB_OBJS)
	mkdir -p $(dir $@)
	$(AR) rcs $@ $(LIB_OBJS)

clean:
	rm -rf $(TMP_OBJ)/lib $(LIB_NAME)

.PHONY: all clean
