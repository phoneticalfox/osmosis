# Paging and Virtual Memory (i386)

This document captures the assumptions and invariants of the initial paging setup for OS/mosis on i386, plus likely failure modes.

## High-level design
- **Identity window:** We identity-map from 0 up to the end of the kernel (rounded up to the nearest page). We cap the identity window to `IDENTITY_MAP_LIMIT` (64 MiB) and never below 16 KiB. This keeps early boot data, VGA text memory, and the kernel image reachable after paging is turned on.
- **Page tables:** The page directory lives in `.bss` and is 4 KiB aligned. Page tables are allocated from the physical frame allocator (PMM) on demand.
- **Heap placement:** The kernel heap starts just past the identity window (but never before `_kernel_end`) to avoid colliding with permanently identity-mapped pages.
- **Basic mapping helpers:** `paging_map`, `paging_unmap`, and `paging_resolve` handle single 4 KiB pages. No large pages are used.

## Invariants
- The page directory is always page-aligned and loaded into CR3 before setting CR0.PG.
- The identity window is contiguous starting at 0 and aligned to 4 KiB.
- All page tables allocated by `paging_map` come from `pmm_alloc_frame` and are zeroed before use.
- `paging_map` refuses to overwrite an existing mapping; callers should unmap first if remapping is required.
- The heap allocator only maps pages above the identity window and never grows past `HEAP_MAX_SIZE` (2 MiB).
- `kmalloc` returns memory aligned to at least 8 bytes; every allocation includes a header so `kfree` can reinsert the block.

## Failure modes to watch for
- **PMM exhaustion:** If `pmm_alloc_frame` returns 0 during paging setup, paging will fail silently and the kernel will likely fault once paging is enabled.
- **Mapping failure in heap growth:** If `ensure_capacity` cannot allocate a frame or map it, the allocator returns `NULL` and the caller must handle it.
- **Double-free or invalid free:** `kfree` ignores pointers outside the heap window, but corrupting the free list (e.g., by scribbling past an allocation) can break future allocations.
- **Page fault handling:** There is no page fault handler yet; any invalid access will triple-fault and reset the system. Keep the identity window and heap mappings consistent.

## Diagnostics
Use the kernel shell commands:
- `paging` to print whether paging is enabled, the CR3 value, and identity map coverage.
- `heap` to show heap bounds, mapped bytes, free-list size, and allocation counters.
- `alloc_test` to run a small allocate-touch-free cycle to sanity-check heap and paging health.
