; OS/mosis GDT Definition
; Correctness First. Clarity Always.

%define KERNEL_CODE_SEG 0x08
%define KERNEL_DATA_SEG 0x10
%define USER_CODE_SEG   0x18
%define USER_DATA_SEG   0x20
%define TSS_SEG         0x28

section .data
align 8
global GDT_START
global GDT_END
global GDTR

GDT_START:
    dq 0x0000000000000000          ; Null descriptor
    dq 0x00CF9A000000FFFF          ; Kernel code: base=0 limit=4GiB
    dq 0x00CF92000000FFFF          ; Kernel data: base=0 limit=4GiB
    dq 0x00CFFA000000FFFF          ; User code:   base=0 limit=4GiB (DPL=3)
    dq 0x00CFF2000000FFFF          ; User data:   base=0 limit=4GiB (DPL=3)
    dq 0x0000000000000000          ; TSS placeholder (patched at runtime)
GDT_END:

GDTR:
    dw GDT_END - GDT_START - 1
    dd GDT_START

section .note.GNU-stack noalloc noexec nowrite align=4
