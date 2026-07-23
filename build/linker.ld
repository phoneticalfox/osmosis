ENTRY(_start)

SECTIONS {
    . = 1M;

    _kernel_start = .;

    .multiboot ALIGN(4) : {
        KEEP(*(.multiboot))
    }

    .text ALIGN(4K) : {
        *(.text*)
    }

    .rodata ALIGN(4K) : {
        *(.rodata*)
        . = ALIGN(4);
        __initramfs_start = .;
        KEEP(*(.initramfs))
        __initramfs_end = .;
    }

    .data ALIGN(4K) : {
        *(.data*)
    }

    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss*)
    }

    _kernel_end = .;
}
