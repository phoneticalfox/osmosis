CROSS   ?=
CC      ?= $(if $(CROSS),$(CROSS)gcc,gcc)
LD      ?= $(if $(CROSS),$(CROSS)ld,ld)
AS      := nasm
ASFLAGS := -f elf32 -Isrc/
CFLAGS  ?= -ffreestanding -std=gnu99 -Wall -Wextra -Iinclude
LDFLAGS ?= -nostdlib -T build/linker.ld
QEMU    ?= ./scripts/qemu.sh
.ONESHELL:

ifeq ($(CROSS),)
# Host toolchain fallback; force 32-bit output when cross tools are unavailable.
CFLAGS  += -m32
LDFLAGS += -m elf_i386
endif

OBJ_DIR      := build/obj
KERNEL_BIN   := build/kernel.bin

BOOT_OBJS    := $(OBJ_DIR)/arch/i386/boot.o
KERNEL_OBJS  := $(OBJ_DIR)/kernel/kernel.o $(OBJ_DIR)/kernel/tty.o \
                $(OBJ_DIR)/kernel/kprintf.o $(OBJ_DIR)/kernel/panic.o \
                $(OBJ_DIR)/kernel/shell.o $(OBJ_DIR)/kernel/boot.o \
                $(OBJ_DIR)/kernel/pmm.o $(OBJ_DIR)/kernel/kmalloc.o \
                $(OBJ_DIR)/kernel/userland.o $(OBJ_DIR)/kernel/process.o \
                $(OBJ_DIR)/kernel/vfs.o \
                $(OBJ_DIR)/arch/i386/idt.o $(OBJ_DIR)/arch/i386/isr_handler.o \
                $(OBJ_DIR)/arch/i386/isr.o $(OBJ_DIR)/arch/i386/irq.o \
                $(OBJ_DIR)/arch/i386/irq_stubs.o $(OBJ_DIR)/arch/i386/pit.o \
                $(OBJ_DIR)/arch/i386/keyboard.o $(OBJ_DIR)/arch/i386/serial.o \
                $(OBJ_DIR)/arch/i386/paging.o $(OBJ_DIR)/arch/i386/tss.o \
                $(OBJ_DIR)/arch/i386/syscall.o $(OBJ_DIR)/arch/i386/syscall_stub.o \
                $(OBJ_DIR)/arch/i386/qemu.o

USER_ELF     := build/user/hello_user.elf
USER_BLOB    := $(OBJ_DIR)/user/hello_user_blob.o
INITRAMFS_BIN := build/initramfs.bin
INITRAMFS_OBJ := $(OBJ_DIR)/initramfs.o

OBJS := $(BOOT_OBJS) $(KERNEL_OBJS) $(USER_BLOB) $(INITRAMFS_OBJ)

all: $(KERNEL_BIN)

$(KERNEL_BIN): $(OBJS) build/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(OBJ_DIR)/arch/i386/boot.o: src/arch/i386/boot/boot.asm src/arch/i386/gdt.asm | $(OBJ_DIR)/arch/i386
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

$(OBJ_DIR)/arch/i386/pit.o: src/arch/i386/pit.c include/osmosis/arch/i386/pit.h include/osmosis/arch/i386/io.h include/osmosis/arch/i386/irq.h | $(OBJ_DIR)/arch/i386
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/i386/keyboard.o: src/arch/i386/keyboard.c include/osmosis/arch/i386/keyboard.h include/osmosis/arch/i386/io.h include/osmosis/arch/i386/irq.h include/osmosis/tty.h | $(OBJ_DIR)/arch/i386
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/i386/serial.o: src/arch/i386/serial.c include/osmosis/arch/i386/serial.h include/osmosis/arch/i386/io.h | $(OBJ_DIR)/arch/i386
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/i386/paging.o: src/arch/i386/paging.c include/osmosis/arch/i386/paging.h include/osmosis/boot.h include/osmosis/pmm.h | $(OBJ_DIR)/arch/i386
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/i386/qemu.o: src/arch/i386/qemu.c include/osmosis/arch/i386/qemu.h include/osmosis/arch/i386/io.h | $(OBJ_DIR)/arch/i386
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/i386/tss.o: src/arch/i386/tss.c include/osmosis/arch/i386/tss.h include/osmosis/arch/i386/segments.h | $(OBJ_DIR)/arch/i386
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/i386/syscall.o: src/arch/i386/syscall.c include/osmosis/arch/i386/syscall.h include/osmosis/arch/i386/segments.h include/osmosis/userland.h | $(OBJ_DIR)/arch/i386
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/i386/syscall_stub.o: src/arch/i386/syscall.asm | $(OBJ_DIR)/arch/i386
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/kernel/%.o: src/kernel/%.c | $(OBJ_DIR)/kernel
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/user/hello_user_blob.o: $(USER_ELF) | $(OBJ_DIR)/user
	objcopy -I binary -O elf32-i386 -B i386 $< $@
	objcopy --add-section .note.GNU-stack=/dev/null --set-section-flags .note.GNU-stack=readonly $@

$(USER_ELF): user/hello_user.c user/linker.ld | build/user
	$(CC) $(CFLAGS) -ffreestanding -fno-pic -fno-pie -fno-stack-protector -nostdlib -no-pie -Wl,-T,user/linker.ld -o $@ $<

$(INITRAMFS_OBJ): $(INITRAMFS_BIN) | $(OBJ_DIR)
	objcopy -I binary -O elf32-i386 -B i386 $< $@
	objcopy --add-section .note.GNU-stack=/dev/null --set-section-flags .note.GNU-stack=readonly $@

$(INITRAMFS_BIN): $(USER_ELF) initramfs/hello.txt | build
	python scripts/mkinitramfs.py $@ $(USER_ELF) initramfs/hello.txt

$(OBJ_DIR):
	@mkdir -p $@

$(OBJ_DIR)/arch/i386: | $(OBJ_DIR)
	@mkdir -p $@

$(OBJ_DIR)/kernel: | $(OBJ_DIR)
	@mkdir -p $@

$(OBJ_DIR)/user: | $(OBJ_DIR)
	@mkdir -p $@

build/user:
	@mkdir -p $@

build:
	@mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(KERNEL_BIN) build/user

qemu: CFLAGS += -DCONFIG_QEMU_EXIT
qemu: clean $(KERNEL_BIN)
	$(QEMU) -kernel $(KERNEL_BIN) -display none -serial stdio -no-reboot -no-shutdown \
	    -device isa-debug-exit,iobase=0xf4,iosize=0x04; \
	status=$$?; \
	[ $$status -eq 0 ] || [ $$status -eq 1 ] || [ $$status -eq 33 ]

.PHONY: all clean
