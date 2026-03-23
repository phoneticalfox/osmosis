OS/mosis Project Structure
The kernel has migrated to a conventional `src/` + `include/` layout, while retaining the earliest single-file kernel snapshot for historical reference.

The active repository root now carries current scaffolding and documentation: identity docs, contributor guidance, build entrypoints, and preserved historical context. It is not a second active home for retired kernel source files.

## Active layout
1. Documentation & philosophy
- `README.md`: Public-facing project overview, current status, and long-term direction.
- `manifesto.md`: The constitutional principles of the system.
- `AGENTS.md`: Guidance for human and automated contributors; preserves intent, wording discipline, architectural truthfulness, and platform-direction discipline.
- `docs/ROADMAP.md`: Strategic sequencing from freestanding kernel seed toward a Unix-derivative future.
- `docs/PLATFORM.md`: Platform and architecture direction from i386 bring-up toward a modern x86_64 daily-driver target.
- `docs/`: Additional roadmap and architecture notes.
2. Boot & hardware (assembly)
- `src/arch/i386/boot/boot.asm`: Stage-0 entry and Protected Mode transition.
- `src/arch/i386/gdt.asm`: Global Descriptor Table definitions.
3. Kernel core (headers)
- `include/osmosis/`: Public kernel headers; architecture headers live under `include/osmosis/arch/i386`.
4. Kernel core (implementation)
- `src/kernel/`: Console, formatting, panic handling, shell glue, PMM, and core kernel services.
- `src/arch/i386/`: IDT, ISR/IRQ glue, PIT heartbeat, keyboard IRQ handling, serial/QEMU helpers, paging, TSS, and syscall plumbing.
5. Build system
- `Makefile`: Freestanding build with generated objects in `build/obj/` and the final image at `build/kernel.bin`.
- `build/linker.ld`: Kernel placement at `0x0010_0000`.

## Legacy snapshot
The frozen single-file kernel snapshot lives under `legacy/osmosis_repo/`. It is preserved for comparison and teaching, but it is not the active build target.

## Directional note
Today, the active tree is a freestanding 32-bit kernel seed with a co-designed kernel/userland philosophy. Long-term, the project direction is toward a Unix-derivative, modern 64-bit daily-driver system with FreeBSD as the intended lineage anchor. That direction should guide roadmap, architecture, and interface decisions, but documentation must not present the current tree as already possessing that maturity before the code and provenance justify the claim.

## Implementation details
All active C components are built with `-ffreestanding -nostdlib`, and assembly sources use NASM. The linker script keeps the kernel start at `0x100000` for common bootloader compatibility.
