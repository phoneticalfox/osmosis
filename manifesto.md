# The OS/mosis Manifesto
*A manifesto for an operating system that refuses to be vague and aims to feel inevitable.*

> **Correctness First. Clarity Always.**  
> If the kernel breathes, it tells you why. If it fails, it fails loudly.

OS/mosis is not “a kernel plus an OS.” It is **one stack, one contract**—co-designed from boot sector to shell prompt so that every byte has an owner, every interrupt has a name, and every promise is testable. This manifesto records the promises we make to ourselves and to anyone who entrusts their time to this project.

---

## I. Prime Directives
1. **Correctness over convenience.** The system prefers a halted CPU to silent corruption. Every subsystem must be testable in isolation and observable in the whole.
2. **Clarity over cleverness.** Interfaces are small, named, and documented. Implementation details stay where they belong; nothing “just happens.”
3. **Freestanding on purpose.** We assume no hosted runtime. Every dependency is deliberate, auditable, and replaceable.
4. **Determinism under stress.** Interrupts, timers, and memory managers must behave the same under load as they do in a demo.

## II. The Panic Covenant
When the kernel cannot uphold the contract, it stops immediately and says so with full context. No mysterious reboots, no quiet retries. A clear panic is honesty; a hidden failure is betrayal.

## III. One Stack, One Surface
- **Unified design.** Kernel and userland evolve together. Syscalls, ABIs, and device models are versioned, documented, and intentionally small.
- **Minimal exposure.** Only the smallest necessary interfaces are exported. Each addition must prove necessity and be described at the point of use.
- **Memory ownership is explicit.** Who allocates, who frees, and when are written down—not implied.

## IV. Boot, Time, and Truth
- **Boot determinism.** The early path does the minimum: enter protected mode, stand up interrupts, stabilize the clock, and speak over serial/tty.
- **Time you can trust.** The PIT heartbeat is the first witness that interrupts remain alive; higher timers build atop it without guesswork.
- **Observability everywhere.** Each subsystem ships with diagnostics: counters, health snapshots, and names for every fault.

## V. Progression Without Sprawl
We advance in phases, never losing the ability to reason about the system:
- Exceptions → Time → Input → Memory → Paging → Heap → Scheduling → User mode → Storage → Shell.
- Each milestone must leave the kernel runnable, debuggable, and explainable to a newcomer with the source tree open.

## VI. Contribution Oath
- **Document intent.** Every change states what was wrong, what was added, and how it is verified.
- **Prefer smaller, finished steps.** Land the simple, correct slice before chasing the ambitious feature.
- **Leave it clearer.** Code and docs should be easier to read after you touch them.
- **Honor the narrative.** Every contribution should reinforce the idea that this is a coherent, deliberate operating system—not a pile of parts.

## VII. The Endgame
Finish Line 1 is “boot to a real shell with persistence.” Finish Lines 2 and 3 add the comfort of tools, networking, and eventually eyes. We will not trade clarity to get there faster; we will get there because clarity lets us build without fear.

## VIII. The Tone
OS/mosis should *feel* inevitable: logs that read like status bulletins, panics that teach, docs that make the path obvious, and tooling that earns trust. The system should look and sound like it knows where it is going—and invite contributors to help steer without losing the map.
