CROSS   ?=
CC      ?= $(if $(CROSS),$(CROSS)gcc,gcc)
LD      ?= $(if $(CROSS),$(CROSS)ld,ld)
AS      := nasm
ASFLAGS := -f elf32 -Isrc/
CFLAGS  ?= -ffreestanding -std=gnu99 -Wall -Wextra -Iinclude
LDFLAGS ?= -nostdlib -T build/linker.ld

ifeq ($(CROSS),)
# Host toolchain fallback; force 32-bit output when cross tools are unavailable.
CFLAGS  += -m32
LDFLAGS += -m elf_i386
endif

OBJ_DIR      := build/obj
KERNEL_BIN   := build/kernel.bin

BOOT_OBJS    := $(OBJ_DIR)/arch/i386/boot.o $(OBJ_DIR)/arch/i386/gdt.o
KERNEL_OBJS  := $(OBJ_DIR)/kernel/kernel.o $(OBJ_DIR)/kernel/tty.o \
                $(OBJ_DIR)/kernel/kprintf.o $(OBJ_DIR)/kernel/panic.o \
                $(OBJ_DIR)/arch/i386/idt.o $(OBJ_DIR)/arch/i386/isr_handler.o \
                $(OBJ_DIR)/arch/i386/isr.o $(OBJ_DIR)/arch/i386/irq.o \
                $(OBJ_DIR)/arch/i386/irq_stubs.o

OBJS := $(BOOT_OBJS) $(KERNEL_OBJS)

all: $(KERNEL_BIN)

$(KERNEL_BIN): $(OBJS) build/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(OBJ_DIR)/arch/i386/boot.o: src/arch/i386/boot/boot.asm src/arch/i386/gdt.asm | $(OBJ_DIR)/arch/i386
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/arch/i386/gdt.o: src/arch/i386/gdt.asm | $(OBJ_DIR)/arch/i386
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/arch/i386/idt.o: src/arch/i386/idt.c include/osmosis/arch/i386/idt.h | $(OBJ_DIR)/arch/i386
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/i386/isr_handler.o: src/arch/i386/isr_handler.c include/osmosis/arch/i386/isr.h | $(OBJ_DIR)/arch/i386
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/i386/isr.o: src/arch/i386/isr.asm | $(OBJ_DIR)/arch/i386
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/arch/i386/irq.o: src/arch/i386/irq.c include/osmosis/arch/i386/irq.h include/osmosis/arch/i386/io.h | $(OBJ_DIR)/arch/i386
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/i386/irq_stubs.o: src/arch/i386/irq.asm | $(OBJ_DIR)/arch/i386
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/kernel/%.o: src/kernel/%.c | $(OBJ_DIR)/kernel
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $@

$(OBJ_DIR)/arch/i386: | $(OBJ_DIR)
	@mkdir -p $@

$(OBJ_DIR)/kernel: | $(OBJ_DIR)
	@mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(KERNEL_BIN)

.PHONY: all clean
