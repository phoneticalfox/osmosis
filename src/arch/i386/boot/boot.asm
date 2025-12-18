; OS/mosis Stage-0 Boot Sequence
%include "arch/i386/gdt.asm"

extern kernel_main

[bits 32]

MULTIBOOT_MAGIC    equ 0x1BADB002
MULTIBOOT_FLAGS    equ 0x0
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .text
global _start

_start:
    cli
    lgdt [GDTR]
    mov eax, cr0
    or al, 0x1
    mov cr0, eax
    jmp KERNEL_CODE_SEG:protected_mode_start

protected_mode_start:
    mov esp, 0x90000
    mov ax, KERNEL_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    call kernel_main

.halt:
    cli
    hlt
    jmp .halt
