# OS/mosis roadmap

This roadmap separates verified implementation from planned direction. Detailed userland sequencing lives in [USERLAND_ROADMAP.md](USERLAND_ROADMAP.md); platform direction lives in [PLATFORM.md](PLATFORM.md).

## Current verified slice

The i386 seed can currently:

- boot through Multiboot and report the supplied memory map;
- install exception and IRQ gates, run a PIT heartbeat, and buffer PS/2 keyboard input;
- allocate physical frames, enable 4 KiB paging, and grow a small kernel heap;
- mount a generated read-only initramfs;
- load one ELF into a dedicated user virtual range;
- cross into ring 3 and service `write`, `exit`, and `getpid` through `int 0x80`;
- resume the kernel after the synchronous user program exits;
- enter an interactive diagnostic shell on normal boots; and
- prove that path headlessly in QEMU with an unambiguous pass/fail exit code.

The tree does **not** currently have a scheduler, a process table, isolated address spaces, `fork`/`execve`/`waitpid`, writable storage, persistence, or a userland shell.

## Near term: make the seed trustworthy

1. **Memory invariants**
   - Add focused PMM tests for unaligned firmware ranges, reserved frames, exhaustion, and invalid frees.
   - Add page-mapping tests that verify user/supervisor and writable/read-only flags.
   - Add heap metadata validation, double-free detection, and coalescing or document why a simpler allocator remains sufficient.

2. **Repeatable boot tests**
   - Emit structured boot-test markers instead of relying only on prose logs.
   - Exercise failure paths for malformed ELF files, invalid user pointers, and unknown syscalls.
   - Keep `make qemu` as the canonical CI and local verification entrypoint.

3. **Finish one process slice**
   - Define address-space ownership and teardown before adding a scheduler.
   - Add one process representation, one scheduling mechanism, and one observable lifecycle test together.
   - Do not reintroduce compile-only `fork`/`execve`/`waitpid` surfaces before they are connected and runnable.

4. **Grow the read-only system surface**
   - Generalize initramfs generation without hiding its simple on-disk format.
   - Move the first shell or utility into userland only when the process and descriptor contracts can support it honestly.

## Mid term: boot to a real userland

5. **Processes and execution**
   - Isolated address spaces, scheduler, `exit`, `waitpid`, and a deliberate spawn or `fork`/`execve` path.
   - Per-process kernel stacks and explicit resource teardown.

6. **Descriptors and files**
   - Descriptor tables, console-backed standard streams, and read-only VFS operations first.
   - A block-device path and a writable filesystem only after ownership and failure behavior are testable.

7. **Userland contract**
   - Minimal libc wrappers generated from or checked against the authoritative syscall definitions.
   - `/bin/sh` plus the smallest utilities needed to prove execution, files, redirection, and error handling.

8. **Persistence and system startup**
   - A real filesystem, an init process, reproducible images, and a documented boot-to-shell chain.

## Long term: platform and lineage transition

9. **x86_64 execution plan**
   - Define the memory model, boot path, calling conventions, interrupt model, and compatibility expectations.
   - Preserve concepts that earned their place in the i386 seed; replace scaffolding that did not.

10. **Truthful Unix-derivative lineage**
    - Define what the FreeBSD lineage anchor means in code and provenance before adopting stronger language.
    - Record what is inspired, independently implemented, ported, or directly derived.

11. **Daily-driver criteria**
    - Stable multitasking, storage, networking, recovery, installation and updates, a coherent user environment, and practical hardware support.
    - The label becomes present tense only when the system can sustain actual daily use.

## Standing rules

- Keep the system runnable and inspectable after every step.
- Land small finished slices instead of broad disconnected scaffolding.
- Use present tense for verified code and future tense for direction.
- Treat i386 as the bring-up runway, not the permanent ceiling.
