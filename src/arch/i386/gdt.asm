; OS/mosis GDT Definition
KERNEL_CODE_SEG equ 0x08
KERNEL_DATA_SEG equ 0x10

section .data
align 8
GDT_START:
    dd 0x0, 0x0                                     ; NULL Descriptor
    dw 0xFFFF, 0x0000, 0x9A00, 0x00CF               ; CODE: Base 0, Limit 4GB, Ring 0
    dw 0xFFFF, 0x0000, 0x9200, 0x00CF               ; DATA: Base 0, Limit 4GB, Ring 0
GDT_END:

GDTR:
    dw GDT_END - GDT_START - 1
    dd GDT_START

section .note.GNU-stack noalloc noexec nowrite align=4
