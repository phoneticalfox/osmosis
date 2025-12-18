# OS/mosis Repository Structure

```
/                    – Top-level build scripts and entrypoints
├── Makefile         – Freestanding build that targets build/kernel.bin
├── build/           – Linker script and generated artifacts
│   ├── linker.ld    – Places the kernel at 0x0010_0000
│   └── obj/         – Object files created by `make`
├── include/         – Public headers
│   ├── osmosis/     – Kernel APIs (tty, printf, panic)
│   └── osmosis/arch/i386 – Architecture-specific APIs (IDT/IRQ/PIT)
├── src/             – Kernel sources
│   ├── arch/i386/   – Platform-specific assembly and C
│   │   ├── boot/    – Entry and early setup
│   │   ├── gdt.asm  – Descriptor table used by the boot path
│   │   ├── idt.c    – Interrupt table configuration
│   │   └── pit.c    – PIT setup + tick accounting for the timer heartbeat
│   └── kernel/      – Platform-agnostic kernel components
│       ├── kernel.c – Kernel entry point and init sequence
│       ├── kprintf.c
│       ├── panic.c
│       └── tty.c
└── docs/            – Reference docs and roadmap
    ├── README.old   – Previous README retained for reference
    └── manifesto.md – OS/mosis principles and milestones
```

Guiding principles:
- Keep architecture-specific code and headers under `arch/<platform>` so future ports stay isolated.
- Keep generated artifacts in `build/` to leave the source tree clean.
- Headers under `include/osmosis` describe the public surface of each subsystem.
