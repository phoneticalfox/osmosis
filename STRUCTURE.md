# OS/mosis repository structure

The active implementation uses one `src/` + `include/` tree. Generated artifacts stay under `build/`, and retired flat-layout sources stay under `legacy/`.

## Active tree

```text
.
├── .github/workflows/   Boot verification and preview artifacts
├── build/
│   └── linker.ld        Tracked kernel linker script; everything else is generated
├── docs/                ABI, memory, platform, and roadmap notes
├── include/osmosis/     Shared and architecture-specific kernel headers
├── initramfs/           Files packed into the read-only boot initramfs
├── scripts/             Build-support and QEMU helpers
├── src/
│   ├── arch/i386/       i386 boot, interrupts, paging, TSS, and syscall entry
│   └── kernel/          Console, memory, shell, VFS, and user-mode loader
├── user/                Current ring-3 demo program and linker script
├── AGENTS.md            Contributor and automation guidance
├── Makefile             Canonical build, clean, and QEMU entrypoints
├── README.md            Current public status and quick start
└── manifesto.md         Project principles
```

## Build flow

1. `user/hello_user.c` is linked as a freestanding i386 ELF at `0x08000000`.
2. `scripts/mkinitramfs.py` packs that ELF and the files under `initramfs/` into `build/initramfs.bin`.
3. GNU `objcopy` wraps the initramfs as an ELF object.
4. NASM and the C compiler produce the kernel objects under `build/obj/`.
5. The linker places the Multiboot kernel at `0x00100000` and embeds the initramfs in `build/kernel.bin`.
6. At boot, the kernel mounts the initramfs, loads `bin/hello_user`, enters ring 3, and returns to either QEMU verification or the interactive kernel shell.

## Boundaries

- Hardware-specific mechanics belong under `src/arch/i386/` with matching headers under `include/osmosis/arch/i386/`.
- Shared kernel contracts belong under `include/osmosis/`; keep that surface smaller than the implementation.
- The current ring-3 launcher is synchronous. A scheduler and multi-process address spaces remain future work and should return only as a complete, testable slice.
- `build/linker.ld` is source despite its directory name. Every other `build/` path is disposable output.

## Historical snapshot

`legacy/osmosis_repo/` preserves the earliest flat-layout version for comparison and teaching. It is deliberately excluded from the active build and should not be used as a second source tree.

## Direction

The active target is a freestanding 32-bit x86 seed. The intended destination is a Unix-derivative, modern 64-bit daily-driver system with FreeBSD as the planned lineage anchor. See [docs/PLATFORM.md](docs/PLATFORM.md) for the boundary between current implementation and future direction.
