# OS/mosis Kernel Seed

OS/mosis is a freestanding 32-bit x86 kernel seed focused on correctness and clarity. This repository contains the minimal pieces needed to enter protected mode, initialize descriptor tables, and provide basic console output for early debugging.

## Layout
- `boot.asm` – 32-bit entry point and protected mode transition.
- `gdt.asm` – Global Descriptor Table definitions.
- `kernel.c` – Kernel entry and integration point for early subsystems.
- `idt.h` / `idt.c` – Interrupt Descriptor Table structures and initialization.
- `tty.h` / `tty.c` – VGA text console driver with scrolling support.
- `kprintf.h` / `kprintf.c` – Minimal printf-style formatter and integer conversion helpers.
- `panic.h` / `panic.c` – Fatal error reporting that halts safely.
- `linker.ld` – Linker script that places the kernel at 0x100000.
- `STRUCTURE.md` – Additional overview of the project structure.
- `manifesto.md` – Philosophical guide for the OS/mosis roadmap.

## Building
1. Install a cross-compilation toolchain (e.g., `i686-elf-gcc`, `i686-elf-ld`, and `nasm`).
2. Assemble the boot and GDT sources:
   ```bash
   nasm -f elf32 boot.asm -o boot.o
   nasm -f elf32 gdt.asm -o gdt.o
   ```
3. Compile the C sources with freestanding flags:
   ```bash
   i686-elf-gcc -ffreestanding -c kernel.c tty.c kprintf.c panic.c idt.c
   ```
4. Link everything into a kernel binary:
   ```bash
   i686-elf-ld -T linker.ld -o kernel.bin boot.o gdt.o kernel.o tty.o kprintf.o panic.o idt.o
   ```
5. Boot the kernel in your emulator of choice, for example with QEMU:
   ```bash
   qemu-system-i386 -kernel kernel.bin
   ```

## Contributing
Follow the principles in `manifesto.md`: prioritize correctness, clarity, and explicit behavior over convenience. Each subsystem should remain testable in isolation and avoid hidden dependencies on hosted runtimes.
