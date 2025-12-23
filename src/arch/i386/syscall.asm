; OS/mosis syscall entry stub (int 0x80)
; Correctness First. Clarity Always.

[BITS 32]

extern syscall_handler

global syscall_stub
syscall_stub:
    cli
    push dword 0          ; dummy error code
    push dword 0x80       ; interrupt number
    pusha

    push ds
    push es
    push fs
    push gs

    mov ax, 0x10          ; kernel data selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call syscall_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds

    popa
    add esp, 8            ; drop err_code and int_no
    sti
    iretd

section .note.GNU-stack noalloc noexec nowrite align=4
