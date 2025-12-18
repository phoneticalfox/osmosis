OS/mosis Project Structure
The following files constitute the initial core of the OS/mosis kernel.
1. Documentation & Philosophy
* manifesto.md: The constitutional principles of the system.
2. Boot & Hardware (Assembly)
* boot.asm: The 32-bit entry point and Protected Mode transition.
* gdt.asm: The memory segment definitions (Global Descriptor Table).
3. Kernel Core (C Headers)
* tty.h: Text console interface.
* kprintf.h: Formatted output interface.
* panic.h: Fatal error handling interface.
* idt.h: Interrupt Descriptor Table structure.
4. Kernel Core (C Implementation)
* kernel.c: The main integration and system entry.
* tty.c: VGA driver and scrolling logic.
* kprintf.c: Printf engine and itoa_base.
* panic.c: Red-screen halt logic.
* idt.c: Interrupt table initialization and management.
5. Build System
* linker.ld: The physical memory map for the kernel binary.
Implementation Details (Summary of Built Components)
All C components are designed to be compiled with -ffreestanding -nostdlib, and assembly files are written for the NASM assembler. The linker.ld ensures the final binary starts exactly at 0x100000 for compatibility with common bootloaders.
Philosophical Compliance: - panic() correctly triggers a hardware halt after reporting errors.
* tty_scroll() handles vertical overflow elegantly.
* kprintf() provides a self-contained formatting engine without external dependencies.