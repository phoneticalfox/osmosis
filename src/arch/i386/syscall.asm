; OS/mosis syscall entry stub (int 0x80)
; Correctness First. Clarity Always.

[BITS 32]

extern syscall_handler

%define USER_CODE_R3 0x1B
%define USER_DATA_R3 0x23

section .text

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
    iretd

; Enter one synchronous ring-3 program. The saved kernel stack makes this
; function return normally once the program calls exit, so the C launcher does
; not need to manufacture a cross-privilege IRET frame in reverse.
global userland_enter
userland_enter:
    push ebp
    push ebx
    push esi
    push edi

    mov [userland_kernel_esp], esp
    pushfd
    pop ecx
    mov [userland_kernel_eflags], ecx
    cli

    mov eax, [esp + 20]   ; entry
    mov edx, [esp + 24]   ; user stack

    mov cx, USER_DATA_R3
    mov ds, cx
    mov es, cx
    mov fs, cx
    mov gs, cx

    push dword USER_DATA_R3
    push edx
    mov ecx, [userland_kernel_eflags]
    or ecx, 0x200         ; user mode starts with interrupts enabled
    push ecx
    push dword USER_CODE_R3
    push eax
    iretd

; A valid exit syscall abandons its ring-3 interrupt frame and restores the
; kernel stack saved by userland_enter.
global userland_resume_kernel
userland_resume_kernel:
    cli
    mov esp, [userland_kernel_esp]
    pop edi
    pop esi
    pop ebx
    pop ebp
    push dword [userland_kernel_eflags]
    popfd
    ret

section .bss
align 4
userland_kernel_esp:    resd 1
userland_kernel_eflags: resd 1

section .note.GNU-stack noalloc noexec nowrite align=4
