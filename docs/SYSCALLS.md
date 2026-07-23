# OS/mosis syscall ABI (i386, `int 0x80`)

This is the ABI implemented by the current synchronous ring-3 demo. It is deliberately smaller than the future process ABI described in [USERLAND_ROADMAP.md](USERLAND_ROADMAP.md).

## Calling convention

- Interrupt vector: `0x80`, installed as a DPL 3 interrupt gate.
- `EAX`: syscall number.
- `EBX`, `ECX`, `EDX`, `ESI`, `EDI`, `EBP`: positional arguments; only the first three are currently used.
- `EAX`: return value for syscalls that resume user mode.
- A normal interrupt return restores the caller's saved flags. The demo launcher begins user mode with IF enabled.

## Error model

Success is non-negative. Failure is a negative numeric errno; there is no userland `errno` variable yet.

- `-9` (`EBADF`) — unsupported descriptor.
- `-14` (`EFAULT`) — invalid or inaccessible user range.
- `-22` (`EINVAL`) — malformed request.
- `-38` (`ENOSYS`) — unknown or reserved syscall.

Invalid requests are logged with the syscall context to keep early ABI failures visible.

## Current table

| Number | Name | Registers | Current behavior |
| ---: | --- | --- | --- |
| 0 | `write` | `EBX=fd`, `ECX=buffer`, `EDX=length` | Supports stdout (`fd=1`) and writes to VGA plus serial after validating every covered page as user-accessible. |
| 1 | `exit` | `EBX=status` | Ends the synchronous demo and restores the suspended kernel stack. A valid call does not return to user mode. |
| 2 | `getpid` | — | Returns `1`, the identity of the single demo task; this is not yet a process-table lookup. |

## Current user mapping

- ELF load base: `0x08000000`.
- Stack range: `0x080FC000` through `0x08100000` (16 KiB, top exclusive).
- Loadable segments must stay below the stack range.
- The loader validates the ELF class, byte order, machine, program-header table, file bounds, memory bounds, and entry point before entering ring 3.
- Pointer-bearing syscalls require the requested range to stay inside the loaded program/stack region and every page to carry `PAGE_USER`.

## Expansion rule

Adding a syscall requires updating the shared numbers, kernel dispatch, user wrapper, this table, and a QEMU test in the same slice. Compile-only syscall surfaces do not count as implementation.
