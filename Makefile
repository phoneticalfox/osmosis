CC      ?= i686-elf-gcc
LD      ?= i686-elf-ld
AS      ?= nasm
CFLAGS  ?= -ffreestanding -std=gnu99 -Wall -Wextra -Iinclude
LDFLAGS ?= -nostdlib -T build/linker.ld

OBJ_DIR      := build/obj
KERNEL_BIN   := build/kernel.bin

BOOT_OBJS    := $(OBJ_DIR)/arch/i386/boot.o $(OBJ_DIR)/arch/i386/gdt.o
KERNEL_OBJS  := $(OBJ_DIR)/kernel/kernel.o $(OBJ_DIR)/kernel/tty.o \
                $(OBJ_DIR)/kernel/kprintf.o $(OBJ_DIR)/kernel/panic.o \
                $(OBJ_DIR)/arch/i386/idt.o

OBJS := $(BOOT_OBJS) $(KERNEL_OBJS)

all: $(KERNEL_BIN)

$(KERNEL_BIN): $(OBJS) build/linker.ld
$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(OBJ_DIR)/arch/i386/boot.o: src/arch/i386/boot/boot.asm src/arch/i386/gdt.asm | $(OBJ_DIR)/arch/i386
$(AS) -f elf32 $< -o $@

$(OBJ_DIR)/arch/i386/gdt.o: src/arch/i386/gdt.asm | $(OBJ_DIR)/arch/i386
$(AS) -f elf32 $< -o $@

$(OBJ_DIR)/arch/i386/idt.o: src/arch/i386/idt.c include/osmosis/arch/i386/idt.h | $(OBJ_DIR)/arch/i386
$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/kernel/%.o: src/kernel/%.c include/osmosis/%.h | $(OBJ_DIR)/kernel
$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/i386:
@mkdir -p $@

$(OBJ_DIR)/kernel:
@mkdir -p $@

clean:
rm -rf $(OBJ_DIR) $(KERNEL_BIN)

.PHONY: all clean
