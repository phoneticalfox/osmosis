# The OS/mosis Manifesto

*Correctness First. Clarity Always.*

1. **Fail visibly.** When the kernel detects an unrecoverable state, it must halt loudly rather than continue silently.
2. **Prefer small, explicit interfaces.** Each subsystem should be understandable without hidden dependencies on hosted runtimes.
3. **Keep code testable in isolation.** Drivers and kernel facilities should remain modular and free of side effects that make them hard to reason about.
4. **Document intent.** Architectural boundaries, calling conventions, and invariants belong in headers and docs so new contributors can navigate confidently.
5. **Optimize later, stabilize first.** Make the simplest safe thing work before adding features or micro-optimizations.
