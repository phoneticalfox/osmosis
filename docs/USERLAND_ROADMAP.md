# Userland roadmap

The userland goal is a coherent Unix-derivative operating environment: a small `/bin/sh` first, then richer tools and eventually zsh with the grml configuration. Kernel interfaces should grow only when a concrete userland slice needs them.

## M0 — synchronous ring-3 proof (current)

Implemented:

- i386 GDT user segments and a dedicated TSS ring-0 stack;
- ELF loading into a fixed user virtual range;
- one generated read-only initramfs containing the demo ELF;
- `int 0x80` with `write`, `exit`, and a single-task `getpid` result;
- user-pointer validation against present user pages; and
- a QEMU test that proves user entry, syscall output, clean exit, and kernel resumption.

Deliberate limitations:

- the launcher is synchronous, not a process scheduler;
- the user mapping lives in the kernel's current page directory;
- there is no descriptor table, libc, persistent filesystem, heap syscall, or ABI-version syscall.

## M1 — one real process lifecycle

Depends on explicit address-space ownership and teardown.

- Give each process its own user mappings and kernel stack.
- Add a small process table with runnable, running, and exited states.
- Schedule at least two deterministic test programs.
- Reap all frames and other resources after exit.
- Keep the current synchronous launcher until the scheduled path passes the same boot test.

Definition of done: two isolated user programs run and exit in QEMU; one program cannot read or write the other's pages; the kernel reports no leaked process resources.

## M2 — execution and waiting

- Choose and document either `spawn` first or the minimum correct `fork` + `execve` path.
- Implement `waitpid` with a real blocked/wakeup state rather than a retry placeholder.
- Copy path and argument data safely across the privilege boundary.
- Reject malformed ELF images without leaving partial mappings behind.

Definition of done: a parent starts a child from the VFS, observes its status, and continues; error paths return stable negative errno values.

## M3 — descriptors and a minimal libc

- Per-process descriptor tables with console-backed stdin/stdout/stderr.
- `read`, `write`, `open`, `close`, `lseek`, and basic `stat`/`fstat` against the VFS.
- Shared syscall-number definitions and checked or generated user wrappers.
- Minimal string, memory, allocation, and error-handling routines.

Definition of done: independently linked utilities can read a file, write output, and report a failed open without kernel-specific helper calls.

## M4 — `/bin/sh`

- A small shell and the utilities it concretely requires (`echo`, `cat`, `true`, `false`, a minimal `test`, and `ls`).
- Current directory support, environment and argument blocks, file redirection, and executable lookup.
- Pipes and `dup2` only when the shell begins using pipelines.

Definition of done: boot → init → `/bin/sh`; `echo hi`, `cat /etc/issue`, a failing command, redirection, and a simple pipeline all behave predictably.

## M5 — terminal and job control

- Signals, process groups, sessions, a controlling terminal, and basic termios.
- `sigaction`, `kill`, `setpgid`, `tcsetpgrp`, and the required `waitpid` states.
- Pseudo-terminals when an actual interactive consumer requires them.

Definition of done: Ctrl+C affects the foreground job, background jobs remain scheduled, and stopped/exited children are reported without races.

## M6 — zsh and a durable user environment

Depends on persistent storage, a mature libc, terminal discipline, environment handling, and enough polling/I/O controls for interactive line editing.

- Package zsh, initially statically if that keeps the dependency story legible.
- Add the grml configuration as an authored system package rather than a build-time accident.
- Preserve history, configuration, and user files across boots.

Definition of done: init launches zsh with the intended prompt; line editing, scripts, foreground/background jobs, and clean shutdown all work.

## ABI rules

- `EAX` selects the syscall; arguments begin in `EBX`, `ECX`, `EDX`, `ESI`, `EDI`, and `EBP`.
- Results are non-negative on success or negative errno on failure.
- Syscall numbers are append-only within an ABI major version.
- The authoritative table remains in one shared definition; docs and user wrappers must not drift from it.
- Every new syscall documents pointer ownership, blocking behavior, failure codes, and which userland feature required it.

## Test strategy

- Use headless QEMU and serial output through the same `make qemu` path used by CI.
- Emit structured markers for each lifecycle event and failure case.
- Give every scenario a timeout so a hang is a test failure.
- Archive the boot log and relevant image hashes for diagnosis.
