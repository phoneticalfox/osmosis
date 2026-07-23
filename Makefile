CROSS ?=

# GNU make defines CC and LD internally, so plain `?=` assignments would leave
# those built-in defaults in place. Respect explicit environment/command-line
# overrides while selecting the matching cross tools when only CROSS is set.
ifeq ($(origin CC), default)
CC := $(if $(CROSS),$(CROSS)gcc,gcc)
endif
ifeq ($(origin LD), default)
LD := $(if $(CROSS),$(CROSS)ld,ld)
endif

OBJCOPY ?= $(if $(CROSS),$(CROSS)objcopy,objcopy)
NASM    ?= nasm
PYTHON  ?= python3
QEMU    ?= ./scripts/qemu.sh
QEMU_TIMEOUT_BIN ?= timeout
QEMU_TIMEOUT     ?= 15s

override CPPFLAGS += -Iinclude
CFLAGS   ?=
LDFLAGS  ?=
ASFLAGS  ?= -f elf32 -Isrc/

COMMON_CFLAGS := -std=gnu99 -Wall -Wextra -ffreestanding -fno-builtin \
                 -fno-pic -fno-pie -fno-stack-protector \
                 -fno-unwind-tables -fno-asynchronous-unwind-tables
KERNEL_LDFLAGS := -nostdlib -T build/linker.ld
USER_LDFLAGS   := -nostdlib -no-pie -Wl,--build-id=none -Wl,-T,user/linker.ld

ifeq ($(CROSS),)
# The host fallback still emits the same 32-bit freestanding objects.
COMMON_CFLAGS  += -m32
KERNEL_LDFLAGS += -m elf_i386
endif

ALL_CFLAGS = $(CPPFLAGS) $(COMMON_CFLAGS) $(CFLAGS)

OBJ_DIR        := build/obj
KERNEL_BIN     := build/kernel.bin
USER_ELF       := build/user/hello_user.elf
INITRAMFS_BIN  := build/initramfs.bin
INITRAMFS_OBJ  := $(OBJ_DIR)/initramfs.o
GENERATED_PATHS := $(OBJ_DIR) $(KERNEL_BIN) $(INITRAMFS_BIN) build/user

BOOT_OBJS := $(OBJ_DIR)/arch/i386/boot.o
KERNEL_OBJS := $(OBJ_DIR)/kernel/kernel.o $(OBJ_DIR)/kernel/tty.o \
               $(OBJ_DIR)/kernel/kprintf.o $(OBJ_DIR)/kernel/panic.o \
               $(OBJ_DIR)/kernel/shell.o $(OBJ_DIR)/kernel/boot.o \
               $(OBJ_DIR)/kernel/pmm.o $(OBJ_DIR)/kernel/kmalloc.o \
               $(OBJ_DIR)/kernel/userland.o $(OBJ_DIR)/kernel/vfs.o \
               $(OBJ_DIR)/arch/i386/idt.o $(OBJ_DIR)/arch/i386/isr_handler.o \
               $(OBJ_DIR)/arch/i386/isr.o $(OBJ_DIR)/arch/i386/irq.o \
               $(OBJ_DIR)/arch/i386/irq_stubs.o $(OBJ_DIR)/arch/i386/pit.o \
               $(OBJ_DIR)/arch/i386/keyboard.o $(OBJ_DIR)/arch/i386/serial.o \
               $(OBJ_DIR)/arch/i386/paging.o $(OBJ_DIR)/arch/i386/tss.o \
               $(OBJ_DIR)/arch/i386/syscall.o $(OBJ_DIR)/arch/i386/syscall_stub.o \
               $(OBJ_DIR)/arch/i386/qemu.o

OBJS := $(BOOT_OBJS) $(KERNEL_OBJS) $(INITRAMFS_OBJ)

all: $(KERNEL_BIN)

$(KERNEL_BIN): $(OBJS) build/linker.ld
	$(LD) $(KERNEL_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(OBJ_DIR)/arch/i386/boot.o: src/arch/i386/boot/boot.asm src/arch/i386/gdt.asm | $(OBJ_DIR)/arch/i386
	$(NASM) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/arch/i386/isr.o: src/arch/i386/isr.asm | $(OBJ_DIR)/arch/i386
	$(NASM) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/arch/i386/irq_stubs.o: src/arch/i386/irq.asm | $(OBJ_DIR)/arch/i386
	$(NASM) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/arch/i386/syscall_stub.o: src/arch/i386/syscall.asm | $(OBJ_DIR)/arch/i386
	$(NASM) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/arch/i386/%.o: src/arch/i386/%.c | $(OBJ_DIR)/arch/i386
	$(CC) $(ALL_CFLAGS) -MMD -MP -c $< -o $@

$(OBJ_DIR)/kernel/%.o: src/kernel/%.c | $(OBJ_DIR)/kernel
	$(CC) $(ALL_CFLAGS) -MMD -MP -c $< -o $@

$(USER_ELF): user/hello_user.c user/linker.ld include/osmosis/syscall_numbers.h | build/user
	$(CC) $(ALL_CFLAGS) $(USER_LDFLAGS) -o $@ $<

$(INITRAMFS_BIN): $(USER_ELF) initramfs/hello.txt scripts/mkinitramfs.py
	$(PYTHON) scripts/mkinitramfs.py $@ $(USER_ELF) initramfs/hello.txt

$(INITRAMFS_OBJ): $(INITRAMFS_BIN) | $(OBJ_DIR)
	$(OBJCOPY) -I binary -O elf32-i386 -B i386 \
		--rename-section .data=.initramfs,alloc,load,readonly,data,contents $< $@
	$(OBJCOPY) --add-section .note.GNU-stack=/dev/null \
		--set-section-flags .note.GNU-stack=readonly $@

$(OBJ_DIR):
	@mkdir -p $@

$(OBJ_DIR)/arch/i386: | $(OBJ_DIR)
	@mkdir -p $@

$(OBJ_DIR)/kernel: | $(OBJ_DIR)
	@mkdir -p $@

build/user:
	@mkdir -p $@

clean:
	rm -rf $(GENERATED_PATHS)

qemu: override CPPFLAGS += -DCONFIG_QEMU_EXIT
qemu: clean $(KERNEL_BIN)
	@set +e; \
	trap 'rm -rf $(GENERATED_PATHS)' EXIT; \
	$(QEMU_TIMEOUT_BIN) $(QEMU_TIMEOUT) $(QEMU) \
		-kernel $(KERNEL_BIN) -display none -serial stdio -no-reboot -no-shutdown \
		-device isa-debug-exit,iobase=0xf4,iosize=0x04; \
	status=$$?; \
	[ $$status -eq 33 ]

-include $(KERNEL_OBJS:.o=.d)

.NOTPARALLEL: qemu
.PHONY: all clean qemu
