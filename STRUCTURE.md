OS/mosis Project Structure
The kernel has migrated to a conventional `src/` + `include/` layout, while retaining the earliest single-file kernel for historical reference.

## Active layout
1. Documentation & philosophy
- `manifesto.md`: The constitutional principles of the system.
- `docs/`: Roadmap and architecture notes.
2. Boot & hardware (assembly)
- `src/arch/i386/boot/boot.asm`: Stage-0 entry and Protected Mode transition.
- `src/arch/i386/gdt.asm`: Global Descriptor Table definitions.
3. Kernel core (headers)
- `include/osmosis/`: Public kernel headers; architecture headers live under `include/osmosis/arch/i386`.
4. Kernel core (implementation)
- `src/kernel/`: Console, formatting, panic handling, shell glue, and PMM scaffolding.
- `src/arch/i386/`: IDT, ISR/IRQ glue, PIT heartbeat, keyboard IRQ handling, serial/QEMU helpers.
5. Build system
- `Makefile`: Freestanding build with generated objects in `build/obj/` and the final image at `build/kernel.bin`.
- `build/linker.ld`: Kernel placement at 0x0010_0000.

## Legacy snapshot
The repository root still contains the original single-file kernel (`boot.asm`, `gdt.asm`, `kernel.c`, `idt.c`, etc.) and a frozen copy under `legacy/osmosis_repo/`. They are not built by default; keep them for comparison or teaching while following the modern layout above.

Implementation details
All active C components are built with `-ffreestanding -nostdlib`, and assembly sources use NASM. The linker script keeps the kernel start at `0x100000` for common bootloader compatibility.
