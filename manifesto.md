# The OS/mosis Manifesto
*A manifesto for an operating system that refuses to be vague and aims to feel inevitable.*

> **Correctness First. Clarity Always.**  
> If the kernel breathes, it tells you why. If it fails, it fails loudly.

OS/mosis is not “a kernel plus an OS.” It is **one stack, one contract**—co-designed from boot sector to shell prompt so that every byte has an owner, every interrupt has a name, and every promise is testable. This manifesto records the promises we make to ourselves and to anyone who entrusts their time to this project.

OS/mosis is also not chasing a merely “Unix-like” costume. The long-term goal is a real Unix-derivative future, with FreeBSD lineage as the intended anchor. The current repository uses a 32-bit seed because narrow bring-up is a good laboratory, not because 32-bit is the intended end state. The destination is a modern 64-bit daily-driver operating system. Until the repository’s code and provenance truly reflect that, we state it as direction—not as a false present-tense claim.

---

## I. Prime Directives
1. **Correctness over convenience.** The system prefers a halted CPU to silent corruption. Every subsystem must be testable in isolation and observable in the whole.
2. **Clarity over cleverness.** Interfaces are small, named, and documented. Implementation details stay where they belong; nothing “just happens.”
3. **Freestanding on purpose.** We assume no hosted runtime. Every dependency is deliberate, auditable, and replaceable.
4. **Determinism under stress.** Interrupts, timers, and memory managers must behave the same under load as they do in a demo.
5. **Truth in lineage.** We distinguish clearly between what the system is today, what it borrows, and what it intends to become.
6. **Bridge without entrenchment.** Near-term scaffolding should help the 64-bit future rather than quietly fossilizing 32-bit assumptions into permanent interfaces.

## II. The Panic Covenant
When the kernel cannot uphold the contract, it stops immediately and says so with full context. No mysterious reboots, no quiet retries. A clear panic is honesty; a hidden failure is betrayal.

## III. One Stack, One Surface
- **Unified design.** Kernel and userland evolve together. Syscalls, ABIs, and device models are versioned, documented, and intentionally small.
- **Minimal exposure.** Only the smallest necessary interfaces are exported. Each addition must prove necessity and be described at the point of use.
- **Memory ownership is explicit.** Who allocates, who frees, and when are written down—not implied.

## IV. Lineage Without Cosplay
- **No fake ancestry.** We do not call OS/mosis a Unix derivative before the codebase and provenance justify that statement.
- **No aesthetic flattening.** “Unix-like” is not enough to describe the project’s long-term goal.
- **Direction matters.** The project is being steered toward a Unix-derivative future with FreeBSD as the intended lineage anchor.
- **Documentation must stay honest.** Present-tense claims describe current reality; future-tense claims describe direction, adoption plans, and architectural intent.

## V. Architecture Horizon
- **The 32-bit seed is the runway.** The current i386 bring-up environment is a proving ground, not the permanent platform ceiling.
- **The 64-bit daily-driver target is real.** The long arc of OS/mosis points toward a modern x86_64 operating system meant for everyday use, not just controlled demos.
- **Interfaces should age well.** New APIs, memory rules, and subsystem boundaries should avoid needless dependence on 32-bit width assumptions when wider and more durable representations are appropriate.
- **Architecture should be layered.** Platform-specific details belong in architecture boundaries, so the kernel core and userland contract can survive platform growth.

## VI. Boot, Time, and Truth
- **Boot determinism.** The early path does the minimum: enter protected mode, stand up interrupts, stabilize the clock, and speak over serial/tty.
- **Time you can trust.** The PIT heartbeat is the first witness that interrupts remain alive; higher timers build atop it without guesswork.
- **Observability everywhere.** Each subsystem ships with diagnostics: counters, health snapshots, and names for every fault.

## VII. Progression Without Sprawl
We advance in phases, never losing the ability to reason about the system:
- Exceptions → Time → Input → Memory → Paging → Heap → Scheduling → User mode → Storage → Shell.
- Each milestone must leave the kernel runnable, debuggable, and explainable to a newcomer with the source tree open.
- Strategic milestones must also preserve the long arc from freestanding seed to documented Unix-derivative, 64-bit daily-driver adoption rather than letting the project drift into a disconnected hobby kernel.

## VIII. Contribution Oath
- **Document intent.** Every change states what was wrong, what was added, and how it is verified.
- **Prefer smaller, finished steps.** Land the simple, correct slice before chasing the ambitious feature.
- **Leave it clearer.** Code and docs should be easier to read after you touch them.
- **Honor the narrative.** Every contribution should reinforce the idea that this is a coherent, deliberate operating system—not a pile of parts.
- **Protect truthfulness.** Do not write docs that overstate lineage, compatibility, provenance, or present-day platform maturity.

## IX. The Endgame
Finish Line 1 is “boot to a real shell with persistence.” Finish Lines 2 and 3 add the comfort of tools, networking, and eventually eyes. Longer-term, OS/mosis aims to mature from freestanding kernel seed into a Unix-derivative, modern 64-bit daily-driver system with explicit lineage decisions, rather than remaining forever a 32-bit bring-up artifact or in the aesthetic category of “Unix-like.” We will not trade clarity to get there faster; we will get there because clarity lets us build without fear.

## X. The Tone
OS/mosis should *feel* inevitable: logs that read like status bulletins, panics that teach, docs that make the path obvious, and tooling that earns trust. The system should look and sound like it knows where it is going—and invite contributors to help steer without losing the map.
