; OS/mosis Stage-0 Boot Sequence
%include "arch/i386/gdt.asm"

extern kernel_main

BITS 32
ORG 0x100000

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
