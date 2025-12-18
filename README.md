# OS/mosis Kernel Layout

OS/mosis is a freestanding 32-bit x86 kernel seed focused on correctness and clarity. This repository now mirrors a more conventional kernel layout with separate source, headers, build artifacts, and documentation.

## Directory overview
- `src/arch/i386/boot/boot.asm` — Stage-0 entry that switches to protected mode and jumps to the kernel.
- `src/arch/i386/gdt.asm` — Global Descriptor Table definition used during the mode switch.
- `src/arch/i386/idt.c` — Interrupt Descriptor Table setup and helpers for exception vectors 0–31.
- `src/arch/i386/isr.asm`, `src/arch/i386/isr_handler.c` — ISR stubs and C handler that print exception context and halt safely.
- `src/arch/i386/irq.asm`, `src/arch/i386/irq.c` — PIC remap, IRQ stubs (32–47), and a basic handler/registration layer.
- `src/arch/i386/pit.c` — PIT configuration and tick counter used to prove interrupts stay alive.
- `src/kernel/` — Core kernel subsystems (console, formatting, panic handling, kernel entry).
- `include/osmosis/` — Public headers for kernel subsystems; architecture-specific headers live under `include/osmosis/arch/i386`.
- `build/linker.ld` — Linker script that places the kernel at 0x0010_0000.
- `build/obj/` — Generated object files (created during the build).
- `docs/` — Additional documentation and roadmap material for OS/mosis.

## Building
You can build with a cross i686-elf toolchain (`nasm`, `i686-elf-gcc`, `i686-elf-ld`) by setting `CROSS=i686-elf-`.

```bash
make CROSS=i686-elf-
```

When a cross toolchain is unavailable, the Makefile falls back to the host toolchain and forces 32-bit output. Install `nasm`
plus 32-bit development support for your platform (e.g., `gcc-multilib` on Debian/Ubuntu) and then run:

```bash
make            # builds build/kernel.bin
```

Artifacts are written under `build/`.

## Running with QEMU
After building:

```bash
qemu-system-i386 -kernel build/kernel.bin
```

## Roadmap position
- Phase A (exceptions) and B1 (PIC remap + IRQ routing) are in place.
- The PIT heartbeat (B2) is configured at 100 Hz and counted during boot to verify interrupts stay alive.
- Next steps on the kernel side: keep the tick counter stable under load and add keyboard IRQ handling to open the door for shell input.

## Notes on organization
- Architecture-specific code is nested under `src/arch/i386` with matching headers in `include/osmosis/arch/i386` to keep future ports contained.
- Common kernel components live under `src/kernel` and include headers from `include/osmosis`.
- The `Makefile` drives the freestanding build and keeps generated objects in `build/obj/` so the source tree stays clean.
