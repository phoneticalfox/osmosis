# OS/mosis Roadmap

This roadmap separates what OS/mosis is now from what it is intended to become.

## Current reality
OS/mosis is currently a freestanding 32-bit x86 kernel seed with a growing kernel/userland contract. It already has real boot, interrupt, timer, input, paging, shell, and user-mode scaffolding work underway, but it is not yet honestly described as a Unix derivative.

## Long-term direction
OS/mosis is intended to become a Unix-derivative operating system with FreeBSD as the intended lineage anchor.

That means:
- the project should not stop at “Unix-like” aesthetics,
- kernel and userland should continue to be co-designed,
- interface choices should increasingly reflect deliberate OS design rather than isolated kernel experimentation,
- provenance and documentation must stay truthful as the project evolves.

## Near-term milestones
1. **Stabilize the kernel seed**
   - Keep boot deterministic.
   - Deepen exception, IRQ, timer, and keyboard confidence.
   - Continue making the shell a trustworthy inspection surface.
2. **Strengthen memory and process foundations**
   - Tighten PMM, paging, heap, and address-space behavior.
   - Continue shaping process and user-mode boundaries.
   - Keep diagnostics strong enough that regressions are legible.
3. **Make userland less symbolic**
   - Expand syscall and ABI documentation.
   - Make the shell and basic utilities feel like a real surface, not just a demo harness.
   - Clarify ownership rules for memory, handles, and process state.
4. **Prepare persistent system surfaces**
   - Mature VFS and storage abstractions.
   - Establish conventions that can survive later Unix-derivative adoption without being throwaway work.

## Mid-term milestones
5. **Coherent OS surface**
   - Evolve the system toward a recognizable operating environment rather than a loose collection of subsystems.
   - Treat shell, filesystem, processes, and userland tools as part of one authored stack.
6. **Truthful lineage planning**
   - Document what “FreeBSD lineage anchor” means in practical terms before stronger claims are made.
   - Be explicit about what is inspired by, what is borrowed from, and what is directly adopted.
   - Avoid drifting into ambiguous phrasing that makes contributors assume compatibility or provenance that does not yet exist.

## Long-term milestones
7. **Unix-derivative transition planning**
   - Move from aspiration to explicit adoption decisions.
   - Record provenance, interface intent, and compatibility goals clearly.
   - Update project language when the technical reality justifies stronger wording.
8. **Real operating system maturity**
   - Persistence, tools, networking, and a more complete user environment.
   - A system that feels coherent because it was developed as one stack, not because unrelated pieces were assembled convincingly.

## Standing rules
- Do not overstate what the current tree is.
- Do not undersell the long-term ambition.
- Use present tense for present reality.
- Use future tense for lineage direction until the repo earns stronger claims.
