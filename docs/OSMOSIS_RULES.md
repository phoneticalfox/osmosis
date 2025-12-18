# OS/mosis OS–Kernel Integration Rules

These rules define how the OS/mosis kernel and the broader OS surface grow together. They are meant to keep the stack tight, understandable, and deliberately integrated—never “a kernel plus a bolted-on OS.”

## 1. One stack, one contract
- Kernel and OS userland are co-designed; any boundary (syscalls, ABI, device models) must be documented and versioned together.
- No opaque shims: everything crossing privilege or process boundaries is named, specified, and testable.

## 2. Correctness first, clarity always
- Prefer simple, explainable mechanisms over cleverness. If it can’t be reasoned about, it doesn’t ship.
- Failure is loud: faults panic or return explicit errors; silent degradation is forbidden.

## 3. Minimal, stable kernel surface
- Expose the smallest syscall surface needed; expand only when userland forces it, and document each addition.
- Keep kernel headers in `include/osmosis` authoritative; any userland bindings trace directly back to them.

## 4. Deterministic boot and init
- Boot performs the minimum work to reach a known-good state (interrupts, console, timers); all optional services start after the system is coherent.
- Init order is explicit, logged, and testable (including failure paths).

## 5. Memory and ownership
- Ownership is explicit: who allocates, who frees, and in what lifetime. No shared, “global” ownership without a stated policy.
- Kernel allocations favor fixed/pooled structures early; general-purpose heaps come with basic integrity checks.

## 6. Interrupt and concurrency discipline
- Interrupt handlers do the minimum: acknowledge, capture, enqueue, and defer. Shared structures touched by IRQs must be guarded (masking or lock-free with documented invariants).
- Preemption-aware code paths clearly state their requirements (e.g., “must run with interrupts disabled”).

## 7. Drivers and devices
- Drivers are deterministic and restartable: init/teardown must be idempotent where possible.
- Device-facing structs and constants live next to their drivers under `include/osmosis/arch/...` to keep hardware knowledge contained.

## 8. Observability and diagnostics
- Every subsystem ships with a minimal diagnostic path (counters, status dumps) that can be compiled in without changing behavior.
- Panics and diagnostics print actionable context (what failed, where, and why).

## 9. Reproducible tooling path
- The build, emulator, and debug tooling are scripted and documented; no hidden local steps.
- CI/automation uses the same entrypoints developers use (`make`, `make qemu`, etc.).

## 10. Compatibility posture
- Target correctness on the reference platform first (i386 + QEMU). Additional platforms or configurations must not silently diverge from the documented contracts.
- Any portability hook is documented at the point of use (why it exists, and the assumptions it preserves).

These rules are living: refine them as subsystems mature, but do not relax correctness or clarity to add features faster.
