ENTRY(_start)
SECTIONS
{
   . = 0x100000;
   .text : { *(.text) }
   .rodata ALIGN (4K) : { *(.rodata) }
   .data ALIGN (4K) : { *(.data) }
   .bss ALIGN (4K) : { *(.bss) }
}