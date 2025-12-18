# OS/mosis Kernel Layout

OS/mosis is a freestanding 32-bit x86 kernel seed focused on correctness and clarity. This repository now mirrors a more conventional kernel layout with separate source, headers, build artifacts, and documentation.

## Directory overview
- `src/arch/i386/boot/boot.asm` — Stage-0 entry that switches to protected mode and jumps to the kernel.
- `src/arch/i386/gdt.asm` — Global Descriptor Table definition used during the mode switch.
- `src/arch/i386/idt.c` — Interrupt Descriptor Table setup and helpers.
- `src/kernel/` — Core kernel subsystems (console, formatting, panic handling, kernel entry).
- `include/osmosis/` — Public headers for kernel subsystems; architecture-specific headers live under `include/osmosis/arch/i386`.
- `build/linker.ld` — Linker script that places the kernel at 0x0010_0000.
- `build/obj/` — Generated object files (created during the build).
- `docs/` — Additional documentation and roadmap material for OS/mosis.

## Building
A cross i686-elf toolchain is required (`nasm`, `i686-elf-gcc`, `i686-elf-ld`).

```bash
make            # builds build/kernel.bin
```

Artifacts are written under `build/`.

## Running with QEMU
After building:

```bash
qemu-system-i386 -kernel build/kernel.bin
```

## Notes on organization
- Architecture-specific code is nested under `src/arch/i386` with matching headers in `include/osmosis/arch/i386` to keep future ports contained.
- Common kernel components live under `src/kernel` and include headers from `include/osmosis`.
- The `Makefile` drives the freestanding build and keeps generated objects in `build/obj/` so the source tree stays clean.
