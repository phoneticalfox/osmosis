# agents.md
## OS/mosis Roadmap (From Kernel Seed to Full Operating System)

> **Correctness First. Clarity Always.**
>
> Roadmap goal: build a complete OS without losing the ability to understand it.

---

## 0. What “Full OS” means for OS/mosis

A “full OS” isn’t one specific feature set. For planning, we define three finish lines:

### Finish Line 1 — Boots to a real shell
- Boots reliably (virtual first; real hardware later)
- Persistent filesystem
- Multiple user programs
- A shell: navigate, list, copy, run programs
- A minimal install/upgrade story for userland components

### Finish Line 2 — Daily-drivable developer OS
- Basic networking
- Enough tools to develop/debug inside the OS
- A simple package/story for installing tools and updates

### Finish Line 3 — Comfort OS (optional)
- Graphics / windowing / fonts / input stack (and maybe audio)
- Not required to be “a full OS”; it’s the “it feels like an OS” expansion

OS/mosis can truthfully call itself “a full OS” at Finish Line 1.

---

## 1. Core constraints (the laws we don’t break)

### Correctness First
- Every subsystem is testable in isolation.
- Failures must be visible, explainable, and halting when unsafe.

### Clarity Always
- Small, clean interfaces.
- Avoid hidden “magic.”
- Prefer fewer features done right over many done vaguely.

### Freestanding kernel
- No hosted libc/runtime assumptions.
- If we need something, we implement it deliberately.

---

## 2. Project tracks (to avoid spaghetti)

- **Track K:** Kernel (interrupts, memory, scheduling, syscalls)
- **Track D:** Drivers (timers, keyboard, disk, PCI, etc.)
- **Track U:** Userland (shell, tools, libc, apps)
- **Track T:** Tooling (build system, debugger workflow, image creation)
- **Track P:** Polish (docs, tests, stability, portability)

---

## Phase A — Truth Under Stress (IDT + ISRs)

### A1. ISR assembly stubs + common exception handler
**Goal:** CPU exceptions (0–31) reliably land in kernel code and produce readable output.

**Definition of done:**
- Intentionally trigger divide-by-zero: prints exception report + halts (no reboot loop).

### A2. Real IDT gates for vectors 0–31
**Definition of done:**
- Multiple exceptions route correctly (e.g., 0, 13, 14).

### A3. Exception naming + minimum register dump
**Definition of done:**
- Panic output includes vector name and at least EIP/CS/EFLAGS (when available).

---

## Phase B — Time + Input (PIC/PIT/Keyboard)

### B1. PIC remap + IRQ stubs + EOI correctness
**Definition of done:**
- Interrupts enabled: system stays alive.

### B2. PIT tick heartbeat
**Definition of done:**
- Tick counter increments stably for 60 seconds without lockups.

### B3. Keyboard scancodes (minimal)
**Definition of done:**
- Keypresses show output reliably under interrupt load.

---

## Phase C — Memory Bedrock (PMM0)

### C1. Choose boot protocol + memory map handoff
**Definition of done:**
- Kernel prints a real memory map table.

### C2. Physical Memory Manager (page frames)
**Definition of done:**
- Allocate/free frames without duplicates or reserved-region corruption.

---

## Phase D — Paging / VMM0 (Virtual Memory)

### D1. Enable paging safely
**Definition of done:**
- Paging on, system still prints; page faults are handled loudly.

### D2. VMM mapping API
**Definition of done:**
- Map/unmap/translate works in controlled tests.

---

## Phase E — Kernel Heap (kmalloc)

### E1. Early allocator (bump) then real heap (free list or slab)
**Definition of done:**
- Kernel can allocate internal structures without chaos.
- Basic heap integrity checks exist (even crude).

---

## Phase F — Multitasking (Threads/Processes)

### F1. Cooperative kernel threads
**Definition of done:**
- Two threads yield back and forth reliably.

### F2. Preemptive scheduling (timer-driven)
**Definition of done:**
- Preemption runs stably for minutes, not seconds.

### F3. Processes (separate address spaces)
**Definition of done:**
- Two user processes run concurrently without accidental memory sharing.

---

## Phase G — Syscalls and User Mode

### G1. Ring 3 entry (GDT/TSS as needed)
**Definition of done:**
- User program starts/returns; cannot scribble over kernel memory.

### G2. Syscall ABI (start simple)
Suggested early syscalls:
- `write` (console)
- `exit`
- `yield` / `sleep`
- `open/read/write/close` (once VFS exists)
- `fork/spawn` (later)

**Definition of done:**
- User program prints via syscall, then exits cleanly.

---

## Phase H — Storage Foundations (VFS + Block I/O)

### H1. Block device abstraction
**Definition of done:**
- Read raw sectors from a disk image (QEMU target is fine).

### H2. VFS skeleton (mounts, nodes)
**Definition of done:**
- Mount a filesystem and list a directory (even if only root).

### H3. First real filesystem
Good early choices:
- FAT (easy to inspect; common in boot contexts)
- ext2 (classic teaching OS filesystem)

**Definition of done:**
- Create/read/write/delete files persistently across reboots.

At this point: you’re approaching Finish Line 1.

---

## Phase I — Executables and “Boot to Shell”

### I1. Program loader (ELF strongly recommended)
**Definition of done:**
- `exec()` loads an ELF user program from the filesystem and runs it.

### I2. `init` process (PID 1 concept)
**Definition of done:**
- Kernel starts init; init starts shell; system doesn’t dead-end in kernel code.

### I3. Basic shell
Minimum commands:
- `help`
- `ls`
- `cd`
- `cat`
- `mkdir` / `rmdir`
- `rm`
- `run` / execute by name
- `reboot` / `shutdown` (even if QEMU-only at first)

**Definition of done:**
- Boot → shell prompt → run programs → return to prompt → files persist.

✅ **Finish Line 1 achieved: “This is a functioning OS.”**

---

## Phase J — Userland Ecosystem (Libc + Tools)

### J1. Minimal libc (just enough)
Start with:
- string/mem routines
- printf-like formatting for userland
- syscall wrappers

### J2. Core utilities
Examples:
- `echo`, `hexdump`, `ps` (once processes exist), `mount`/`umount` (later)

**Definition of done:**
- You can “live” in the OS without feeling trapped.

---

## Phase K — Drivers That Make It Feel Real

### K1. PCI enumeration
**Definition of done:**
- Kernel lists PCI devices reliably.

### K2. Better disk path
Targets:
- virtio-blk (great for QEMU), IDE (simple), AHCI (modern)

### K3. Better input stack
- scancode → key events
- layouts, modifiers, repeat

---

## Phase L — Networking (optional, but powerful)

### L1. NIC driver target (virtio-net is ideal for QEMU)
**Definition of done:**
- Link up, send/receive frames.

### L2. Minimal stack order
- ARP → IPv4 → ICMP (ping) → UDP → TCP (later)

### L3. DHCP (optional)
**Definition of done:**
- Auto-config on common virtual networks.

---

## Phase M — Security Model (even if simple)

### M1. Users/groups/permissions (or a capability model)
**Definition of done:**
- Kernel enforces file and process permissions.

### M2. Process isolation hardening
**Definition of done:**
- Common userland crashes don’t corrupt kernel state.

---

## Phase N — Graphics (if/when OS/mosis wants eyes)

### N1. Framebuffer driver + font rendering
**Definition of done:**
- Draw pixels and render text reliably.

### N2. Windowing/compositor (optional)
### N3. UI toolkit (optional)

Finish Line 3 territory: “Comfort OS.”

---

## Phase O — Portability and architecture decisions

### O1. Decide: 32-bit x86 forever, or graduate to x86_64
Sane path:
- Hit Finish Line 1 on 32-bit first.
- Port to x86_64 as a dedicated milestone.

---

## Phase P — Distribution and “It Lives”

### P1. Reproducible image builder
**Definition of done:**
- A script produces a bootable image with kernel + userland + FS.

### P2. Package format (keep it tiny)
- Bundles + manifest + checksum is enough to start.

### P3. Documentation
- Build, run, architecture overview.

---

## 3. Suggested version targets

- **v0.2 — Truth:** exceptions handled, no invisible reboots
- **v0.3 — Time & Input:** timer + keyboard IRQ stable
- **v0.4 — Memory:** PMM + paging + heap stable
- **v0.5 — Multitasking:** preemptive threads stable
- **v0.6 — User mode:** syscalls + hello-from-userland
- **v0.7 — Disk + FS:** persistence
- **v0.8 — Boot to shell:** Finish Line 1
- **v0.9 — Tools + stability**
- **v1.0 — “Full OS”:** shell, FS, multitasking, tools, reproducible builds

---

## 4. Correctness acceptance tests (expanded)

Truth tests:
- Divide-by-zero prints and halts
- Page fault prints and halts (once paging exists)
- Interrupts enabled never causes reboot loop

Time tests:
- Timer tick stable under load

Process tests:
- Two user processes run concurrently
- One process crashing does not kill the system

Filesystem tests:
- Create/read/write/delete persists across reboot
- Basic corruption detection catches obvious damage

Exec tests:
- Run program from shell, return to prompt

Reproducibility tests:
- Clean build produces the same bootable image structure every time
