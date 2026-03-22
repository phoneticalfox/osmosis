# OS/mosis Platform Direction

This document records the architecture and platform direction of OS/mosis so contributors and agents do not confuse the current bring-up environment with the intended destination.

## Current reality
OS/mosis currently builds and runs as a freestanding 32-bit x86 kernel seed.

That choice is useful because it keeps early boot, interrupts, paging, diagnostics, and core kernel reasoning narrow enough to understand while the system is still finding its shape.

## Intended destination
OS/mosis is intended to become a modern 64-bit operating system suitable to grow toward daily-driver use.

For platform direction, that currently implies:
- x86_64 as the long-term architecture target,
- a kernel and userland contract that does not assume 32-bit limits forever,
- architecture boundaries that let early i386 work inform the future instead of trapping it.

## What “32-bit seed” means
The current i386 implementation is:
- a bring-up laboratory,
- a proving ground for interrupt, memory, shell, and process ideas,
- a place to make mistakes while the conceptual surfaces are still small.

It is **not** the intended permanent ceiling of the project.

## What “64-bit daily driver” means
This phrase should not remain poetic fog. At minimum, it points toward:
- a real 64-bit address model,
- memory and process abstractions that survive beyond toy limits,
- storage and filesystem behavior meant for actual sustained use,
- networking and userland tooling that support everyday work,
- a system stability story strong enough that using it daily would not be a stunt.

The repo is not there yet, so the phrase remains directional. But it should shape choices now.

## Design rules implied by this direction
- Avoid cementing needless 32-bit assumptions into core interfaces.
- Prefer `size_t`, `uintptr_t`, and semantically appropriate fixed-width integers over casually using `uint32_t` everywhere.
- Keep architecture-specific mechanics behind architecture boundaries.
- Separate “temporary bring-up simplification” from “core operating-system truth.”
- When adding new abstractions, ask whether they survive the x86_64 future with only local changes.

## Good intermediate outcomes
Even before a real x86_64 transition lands, the project can still move in the right direction by:
- reducing width-sensitive assumptions in shared headers,
- clarifying ownership and address-space contracts,
- documenting boot and platform assumptions explicitly,
- avoiding public interfaces that only make sense in a narrow i386 world.

## Truthfulness rule
Do not describe OS/mosis as already being a 64-bit daily-driver OS.

Describe it as:
- a current 32-bit seed,
- with a long-term x86_64 daily-driver direction,
- and architecture guidance meant to keep today’s work from blocking tomorrow’s platform.
