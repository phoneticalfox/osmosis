#include <stddef.h>
#include <stdint.h>

#include "osmosis/kmalloc.h"
#include "osmosis/arch/i386/paging.h"
#include "osmosis/kprintf.h"
#include "osmosis/pmm.h"

#define HEAP_MAX_SIZE (2u * 1024u * 1024u) /* 2 MiB heap */
#define HEAP_ALIGNMENT 8u

extern char _kernel_end[];

struct heap_block {
    size_t size; /* total bytes including header */
    struct heap_block *next;
};

static uintptr_t heap_base = 0;
static uintptr_t heap_limit = 0;
static uintptr_t heap_top = 0;
static uintptr_t heap_mapped_end = 0;
static struct heap_block *free_list = NULL;
static uint32_t total_allocs = 0;
static uint32_t total_frees = 0;

static inline uintptr_t align_up(uintptr_t value, uintptr_t align) {
    return (value + align - 1u) & ~(align - 1u);
}

static int ensure_capacity(uintptr_t new_top) {
    if (new_top > heap_limit) {
        return 0;
    }

    while (heap_mapped_end < new_top) {
        uintptr_t frame = pmm_alloc_frame();
        if (!frame) {
            return 0;
        }

        if (!paging_map(heap_mapped_end, frame, PAGE_WRITE)) {
            return 0;
        }

        for (uintptr_t *ptr = (uintptr_t *)heap_mapped_end;
             ptr < (uintptr_t *)(heap_mapped_end + PAGE_SIZE);
             ptr++) {
            *ptr = 0;
        }

        heap_mapped_end += PAGE_SIZE;
    }

    return 1;
}

void kmalloc_init(void) {
    struct paging_stats pstats = paging_get_stats();
    uintptr_t end = align_up((uintptr_t)_kernel_end, PAGE_SIZE);
    heap_base = align_up(pstats.identity_limit, PAGE_SIZE);
    if (heap_base < end) {
        heap_base = end;
    }
    heap_limit = heap_base + HEAP_MAX_SIZE;
    heap_top = heap_base;
    heap_mapped_end = heap_base;

    if (!ensure_capacity(heap_base + PAGE_SIZE)) {
        kprintf("kmalloc: failed to map initial page\n");
    } else {
        heap_top = heap_base;
    }
}

static struct heap_block **find_suitable_block(size_t needed) {
    struct heap_block **cursor = &free_list;
    while (*cursor) {
        if ((*cursor)->size >= needed) {
            return cursor;
        }
        cursor = &(*cursor)->next;
    }
    return NULL;
}

static void split_block(struct heap_block *block, size_t needed) {
    if (block->size <= needed + sizeof(struct heap_block) + HEAP_ALIGNMENT) {
        return;
    }

    uintptr_t block_addr = (uintptr_t)block;
    uintptr_t next_addr = block_addr + needed;
    struct heap_block *next = (struct heap_block *)next_addr;
    next->size = block->size - needed;
    next->next = block->next;

    block->size = needed;
    block->next = next;
}

static void *alloc_from_free(size_t total_size) {
    struct heap_block **location = find_suitable_block(total_size);
    if (!location) {
        return NULL;
    }

    struct heap_block *block = *location;
    split_block(block, total_size);
    *location = block->next;

    return (void *)((uintptr_t)block + sizeof(struct heap_block));
}

static void *alloc_from_bump(size_t total_size) {
    uintptr_t aligned_top = align_up(heap_top, HEAP_ALIGNMENT);
    uintptr_t new_top = aligned_top + total_size;

    if (!ensure_capacity(new_top)) {
        return NULL;
    }

    struct heap_block *block = (struct heap_block *)aligned_top;
    block->size = total_size;
    block->next = NULL;

    heap_top = new_top;
    return (void *)((uintptr_t)block + sizeof(struct heap_block));
}

void *kmalloc(size_t size) {
    if (!size) {
        return NULL;
    }

    size_t total_size = align_up(size + sizeof(struct heap_block), HEAP_ALIGNMENT);

    void *ptr = alloc_from_free(total_size);
    if (!ptr) {
        ptr = alloc_from_bump(total_size);
    }

    if (ptr) {
        total_allocs++;
    }
    return ptr;
}

static void insert_free_block(struct heap_block *block) {
    block->next = free_list;
    free_list = block;
}

void kfree(void *ptr) {
    if (!ptr) {
        return;
    }

    uintptr_t addr = (uintptr_t)ptr - sizeof(struct heap_block);
    if (addr < heap_base || addr >= heap_limit) {
        return;
    }

    struct heap_block *block = (struct heap_block *)addr;
    insert_free_block(block);
    total_frees++;
}

struct kmalloc_stats kmalloc_get_stats(void) {
    struct kmalloc_stats stats;
    stats.heap_base = heap_base;
    stats.heap_limit = heap_limit;
    stats.heap_top = heap_top;
    stats.mapped_bytes = heap_mapped_end - heap_base;

    uintptr_t free_bytes = 0;
    struct heap_block *cursor = free_list;
    while (cursor) {
        free_bytes += cursor->size;
        cursor = cursor->next;
    }
    stats.free_bytes = free_bytes;
    stats.total_allocs = total_allocs;
    stats.total_frees = total_frees;
    return stats;
}
