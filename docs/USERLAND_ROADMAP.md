# Userland Roadmap

Goal: deliver a Unix-like userland on OS/mosis with `/bin/sh` first, and zsh (with grml config) later. Correctness first, clarity always; keep the kernel surface minimal and documented.

## Milestones (dependencies & definition of done)

- **M0 — User/kernel contract frozen**
  - **Depends on:** Ring 3 entry, stable IDT/GDT/TSS, timer + keyboard IRQ, basic physical/virtual memory, kernel heap.
  - **Work:** Select syscall ABI (see below); document all syscalls; provide kernel stubs for the initial set; add a user-mode “hello” that exits via syscall; add a serial/logging convention for automated tests.
  - **Definition of done:** QEMU boots a user binary that issues at least `write` + `exit` via the ABI; `get_abi_version` (or equivalent) reports the negotiated version; boot log shows the ABI identifier.

- **M1 — Minimal POSIX slice for `/bin/sh`**
  - **Depends on:** M0; ELF loader for user programs; minimal libc wrappers.
  - **Work:** Implement the “POSIX slice (sh)” syscalls below; ship `/bin/sh` (small POSIX-ish shell) and the few core utilities it depends on (`echo`, `cat`, `true/false`, `test` minimal, `ls` optional at first); ensure `fork`/`execve`/`waitpid` path is stable; support basic redirection without pipelines.
  - **Definition of done:** Boot → `/bin/sh` prompt on console/serial; run `echo hi`, `cat /etc/issue` (or similar) succeeds; `sh -c "exit 0"` returns status 0 to caller; negative errno model observed when syscalls fail (and surfaced via libc).

- **M2 — Pipes and redirection completeness**
  - **Depends on:** M1; working pipe/dup2; file descriptors for stdio wired through the console driver.
  - **Work:** Implement `pipe`, `dup2`, `close` correctness; allow `cmd1 | cmd2` and `cmd >file` in `/bin/sh`; add lightweight file status (`stat/fstat/lseek`) for redirection sanity checks; ensure `errno` thread safety is defined (per-thread copy once threads exist).
  - **Definition of done:** In QEMU, `echo hi | cat` prints `hi`; `cat /nope` returns non-zero with `errno` set; boot log captures pipeline success markers.

- **M3 — Job control + terminal discipline**
  - **Depends on:** M2; signals; process groups; controlling terminal rules; `termios` basics.
  - **Work:** Add `fork`, `setpgid`, `tcsetpgrp`, `tcgetattr/tcsetattr` (canonical + raw), `sigaction`, `kill`, `sigsuspend`, `waitpid` with `WUNTRACED`/`WCONTINUED`; support `SIGINT/SIGTSTP/SIGCHLD`; document signal delivery semantics; add `pty` minimal layer for future interactive shells.
  - **Definition of done:** `/bin/sh` supports Ctrl+C to interrupt foreground jobs and `sleep 1 &` continues; stopped jobs can be foregrounded with `fg`; boot log records job transitions without kernel crashes.

- **M4 — zsh (with grml config)**
  - **Depends on:** M3; filesystem persistence for `/etc` and `$HOME`; dynamic memory in userland (malloc/free); enough `ioctl`/`termios` for line editing; environment variable plumbing.
  - **Work:** Package zsh with grml config (possibly statically linked initially); ensure `poll`/`select` or `pselect` exists for interactive editing; provide `isatty`, `ttyname` helpers; verify prompt rendering and history; tune resource limits (`getrlimit/setrlimit` optional but planned).
  - **Definition of done:** Boot → init → zsh launches with grml prompt; line editing works; background/foreground jobs behave; simple scripts execute; boot log includes zsh startup and clean exit markers.

## Syscall ABI proposal (i386)

- **Entry mechanism:** Use `int 0x80` initially for debuggability and compatibility with early boot stages. Keep the door open for `sysenter` once the ABI stabilizes; add a feature flag in `get_abi_version` to advertise which entry points are available/enabled.
- **Calling convention:**  
  - `EAX` = syscall number.  
  - `EBX`, `ECX`, `EDX`, `ESI`, `EDI`, `EBP` = up to six arguments (ordered).  
  - Scratch: `EAX`, `EDX`, `ECX`; preserved: `EBX`, `ESI`, `EDI`, `EBP` (unless used for args).  
  - Return: `EAX` = success value (≥ 0) or negative errno (`-E...`). `EDX` may carry high bits for 64-bit results when needed (documented per syscall).
- **Naming conventions:** Kernel handlers as `sys_<name>`; user-facing wrappers as plain names in libc. Enumerate syscall numbers in a single header (`include/sys/syscall.h`-style) and generate userland constants from the same source to avoid drift.
- **Error model:** Always negative errno in `EAX`; libc converts to `-1` + `errno` for POSIX APIs. Undefined behavior is avoided: invalid pointers return `-EFAULT`, bad fds `-EBADF`, unsupported ops `-ENOSYS`.
- **Versioning strategy:**  
  - `get_abi_version` syscall returns `{major, minor, flags}`; major bumps for breaking changes, minor for additive.  
  - Keep syscall numbers stable; new syscalls are appended only.  
  - Deprecations are flagged in `flags` and remain callable until the next major bump.  
  - Boot log must print the ABI tuple and entry method in use.

## Minimal POSIX slices

- **Slice A — `/bin/sh` essentials (no job control yet)**  
  `read`, `write`, `open/close`, `lseek`, `fstat/stat`, `unlink`, `chdir`, `getcwd` (optional but helpful), `fork`, `execve`, `_exit/exit`, `waitpid`, `pipe`, `dup/dup2`, `umask` (for redirection), `getpid`, `brk`/`sbrk` or `mmap` for allocator, `gettimeofday` (for shell `time` or arithmetic), `isatty` helper in libc. Signals: `sigaction` + `SIGINT` handling sufficient to terminate foreground commands cleanly.

- **Slice B — zsh + grml needs (superset)**  
  All of Slice A plus: `sigprocmask`, `sigsuspend`, `setpgid`, `tcsetpgrp/tcgetpgrp`, `kill`, `waitpid` with job-control flags, `getppid`, `setsid` (if implementing a session model), `ioctl` for termios (`TCGETS/TCSETS*`), `poll`/`select`/`pselect`, `getrlimit/setrlimit` (optional but helpful), `pipe2` (non-blocking/close-on-exec variants) or fcntl flags, `dup3` (optional), pseudo-terminals, environment block support (`environ`, `setenv/unsetenv`, `putenv`), `errno` thread-safety guarantee for future pthreads, and stable `/dev/null`, `/dev/tty`.

## Test strategy (QEMU + boot.log)

1. **Image build:** Produce a bootable disk image containing kernel + userland slices. Embed a small test init that runs scripted scenarios for each milestone (e.g., write/exit for M0, `echo hi | cat` for M2, job control probes for M3).
2. **Execution harness:** Run headless QEMU with serial redirected to a file:  
   `qemu-system-i386 -m 256M -kernel kernel.elf -serial file:boot.log -no-reboot -no-shutdown -monitor none -display none [...]`  
   Add `-append "test_scenario=<name>"` if using multiboot cmdline parsing.
3. **Pass/fail parsing:** Host-side script parses `boot.log` for structured markers (`[TEST] name=... result=PASS/FAIL errno=<n>`). Non-zero QEMU exit or missing PASS markers = failure. Keep per-test timeouts to catch hangs.
4. **Incremental coverage:**  
   - M0: verify `write` output + clean `_exit`.  
   - M1: run `/bin/sh -c "exit 0"`; check exit code emitted to log.  
   - M2: exercise `pipe` + `dup2` with `echo hi | cat`.  
   - M3: spawn background job, send `SIGINT`, verify foreground job stops; check `SIGCHLD` reaping.  
   - M4: launch zsh, run a simple script, confirm prompt and exit path.  
5. **Artifacts:** Archive `boot.log`, kernel/userland hashes, and ABI version for each run; keep them in CI artifacts for regressions.

## Notes on keeping the kernel surface minimal

- Only add syscalls when a specific userland feature needs them; document each syscall with purpose, args, errno, and side effects.
- Prefer composable primitives (`pipe`, `dup2`, `read`/`write`, `sigaction`) over high-level helpers. If a high-level helper is added (e.g., `spawn`), ensure it can be emulated by libc once richer POSIX APIs exist.
- Reuse existing drivers (console/serial) for early userland I/O to avoid premature terminal complexity; introduce PTYs only when job control demands it.
