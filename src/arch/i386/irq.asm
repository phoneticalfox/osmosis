; OS/mosis IRQ stubs for hardware interrupts (vectors 32-47)
; Correctness First. Clarity Always.

[BITS 32]

extern irq_handler

%macro IRQ_STUB 2
    global irq%1
irq%1:
    cli
    push dword 0          ; dummy error code
    push dword %2         ; interrupt number
    jmp irq_common_stub
%endmacro

IRQ_STUB 0, 32
IRQ_STUB 1, 33
IRQ_STUB 2, 34
IRQ_STUB 3, 35
IRQ_STUB 4, 36
IRQ_STUB 5, 37
IRQ_STUB 6, 38
IRQ_STUB 7, 39
IRQ_STUB 8, 40
IRQ_STUB 9, 41
IRQ_STUB 10, 42
IRQ_STUB 11, 43
IRQ_STUB 12, 44
IRQ_STUB 13, 45
IRQ_STUB 14, 46
IRQ_STUB 15, 47

global irq_common_stub
irq_common_stub:
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

    push esp              ; pass pointer to stack frame
    call irq_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds

    popa
    add esp, 8            ; pop err_code and int_no
    sti
    iretd

section .note.GNU-stack noalloc noexec nowrite align=4
