# AGENTS.md

This file preserves OS/mosis project intent for both human contributors and automated agents.

## Project identity
OS/mosis is one stack, one contract: kernel and userland are meant to be developed together, not treated as unrelated layers glued together after the fact.

Today, the repository is a freestanding 32-bit x86 kernel seed with an emerging userland surface.

Long-term, the project is intended to grow toward a real Unix-derivative, modern 64-bit daily-driver system with FreeBSD as the intended lineage anchor. Those statements are directional unless and until the codebase and provenance justify stronger present-tense wording.

## Documentation truthfulness rules
- Do not claim OS/mosis is already a Unix derivative unless the code and provenance support that literally.
- Do not claim OS/mosis is already a modern 64-bit daily-driver OS unless the implementation supports that literally.
- Do not reduce the project’s goal to a merely “Unix-like” aesthetic.
- Prefer wording like “current seed,” “intended direction,” “long-term goal,” “planned lineage anchor,” or “future platform target” when discussing 64-bit and FreeBSD-derived direction before those realities are materially present in the repo.
- Present-tense docs should describe what exists now. Future-tense docs should describe what the project is steering toward.

## Contribution priorities
1. Keep the system runnable, inspectable, and debuggable after each step.
2. Prefer small, finished slices over broad speculative scaffolding.
3. Preserve coherence between kernel interfaces, userland expectations, docs, tooling, and long-term platform direction.
4. Leave code and docs clearer than you found them.
5. When in doubt, choose correctness and explicitness over convenience.

## Architectural guidance
- Preserve the “one stack, one contract” model.
- Treat syscall, ABI, VFS, process, shell, and tooling decisions as part of a co-designed OS surface, not just kernel internals.
- Keep architecture-specific assumptions behind architecture boundaries when possible.
- Avoid accidental sprawl in public interfaces.
- Keep observability high: logs should teach, panics should explain, and docs should make the next step obvious.
- Do not introduce lineage language casually; provenance claims must be deliberate.
- Do not quietly entrench 32-bit assumptions into interfaces that are meant to survive into a 64-bit future.
- Prefer width-resilient concepts and types where appropriate: `size_t`, `uintptr_t`, explicit fixed-width integers when semantically correct, and architecture-specific wrappers where needed.
- Treat i386 as the bring-up runway, not the permanent ceiling.

## Repo hygiene
- The active implementation lives under `src/` and `include/`.
- Historical single-file artifacts belong in `legacy/osmosis_repo/`, not in the active root.
- Root-level documentation should reflect current reality and current direction.
- New docs that shape platform or architecture direction should live in-repo so future agents do not have to reconstruct project intent from external conversation.

## Good changes for agents to make
- Tighten docs so they better reflect current architecture, future direction, and platform intent.
- Update roadmap and platform sequencing when major milestones shift.
- Clarify comments, headers, diagnostics, and verification notes.
- Land small, verifiable infrastructure improvements that support later OS work.
- Reduce needless 32-bit lock-in in interfaces when doing so does not misrepresent present implementation status.

## Changes to avoid
- Making unsupported compatibility, lineage, platform-maturity, or daily-driver claims.
- Adding large speculative subsystems without a clear verification story.
- Letting docs drift out of sync with the actual tree.
- Treating OS/mosis like a generic hobby kernel when the project’s identity is more deliberate than that.
- Cementing narrow i386-only assumptions in core interfaces without a very good reason.
