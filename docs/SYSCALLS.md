# OS/mosis Syscall ABI (i386, `int 0x80`)

## Calling convention
- Interrupt vector: **0x80** (DPL=3 gate in the IDT).
- Calling convention: Linux-style registers.
  - **EAX**: syscall number (see table below).
  - **EBX, ECX, EDX, ESI, EDI, EBP**: positional arguments (only the first three are used today).
  - **EAX**: return value.
- Flags: the kernel forces IF=1 on return from a syscall.

## Error model
- Returns `>= 0` on success.
- Returns **negative errno** on failure. Errno values are numeric only (there is no per-process `errno` variable yet).
  - `9`  (`-EBADF`)   – bad/unsupported descriptor.
  - `14` (`-EFAULT`)  – invalid user pointer or unmapped page.
  - `22` (`-EINVAL`)  – malformed request (e.g., null buffer).
  - `38` (`-ENOSYS`)  – syscall not implemented.
- The kernel logs loud failure context for invalid requests (number, EAX, and EIP).

## Syscall table

| Number | Name    | Registers                     | Notes |
| ------ | ------- | ----------------------------- | ----- |
| 0      | `write` | EBX=fd, ECX=buf, EDX=len      | Only `fd=1` (console) is supported. Copies directly from user pages; range-checked for user accessibility. |
| 1      | `exit`  | EBX=exit_code                 | Terminates the current user program and returns to the kernel launcher. |
| 2      | `getpid`| –                             | Stub; always returns `1`. |
| 3      | `brk`   | EBX=new_break                 | Placeholder; always `-ENOSYS`. |

## User program expectations
- User pages live at 0x04000000 and above; the loader maps the ELF segments and a 16 KiB user stack at 0x04100000.
- The kernel validates pointers against mapped user pages (`PAGE_USER`); unmapped or supervisor-only pointers fail with `-EFAULT`.
- Syscall surface is intentionally minimal; expanding it requires updating this table and the shared `include/osmosis/syscall_numbers.h`.
