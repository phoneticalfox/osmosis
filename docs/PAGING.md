# Paging and Virtual Memory (i386)

This document records the paging model implemented by the current i386 seed. It describes the active code, not a future multi-process design.

## Current layout

- **Identity window:** virtual address zero through the highest whole page below `identity_limit` maps to the same physical addresses. The limit normally follows detected usable memory up to a 64 MiB cap, while always covering the kernel and enough low memory to allocate the page tables used during setup.
- **Kernel heap:** the heap begins at or above `identity_limit`, never before `_kernel_end`, and may grow by at most 2 MiB. Heap pages receive physical frames on demand.
- **User demo:** the synchronous ring-3 ELF is loaded at `0x08000000`; its 16 KiB stack occupies `0x080fc000` through `0x08100000`. This window is deliberately separate from the kernel heap.
- **Page tables:** the page directory is a page-aligned kernel object. Page tables are allocated from physical frames that remain reachable through the identity window.

The kernel currently uses one shared page directory. There is no address-space creation, switching, isolation between processes, or copy-on-write.

## Mapping contract

- Pages are 4 KiB; large pages are not used.
- `paging_map` and `paging_unmap` accept page-aligned addresses and operate on the active directory.
- `paging_map` refuses to replace an existing mapping.
- `paging_resolve` translates one virtual address in the active directory.
- `paging_range_has_flags` verifies that an entire byte range is mapped with the requested user/write permissions. The syscall layer uses it before reading user memory.
- Page-directory permissions are promoted when a later mapping needs user or write access.

## Failure behavior

- Failure to construct the initial identity map is fatal and produces a kernel panic.
- Heap growth returns failure if it cannot allocate or map a frame; a frame allocated immediately before a failed mapping is released.
- Invalid CPU accesses reach the generic exception handler, which prints the vector and register context before panicking. There is not yet a demand-paging or recovery path.
- `kfree` rejects pointers outside the heap window, but it does not yet detect double frees or coalesce free blocks.

## Diagnostics

The kernel shell provides:

- `paging` for paging state, CR3, identity coverage, mapped-page count, and table count.
- `heap` for heap bounds, mapped bytes, free-list bytes, and allocation counters.
- `alloc_test` for a small allocate, touch, and free check.
