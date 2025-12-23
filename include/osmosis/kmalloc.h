#ifndef OSMOSIS_KMALLOC_H
#define OSMOSIS_KMALLOC_H

#include <stddef.h>
#include <stdint.h>

struct kmalloc_stats {
    uintptr_t heap_base;
    uintptr_t heap_limit;
    uintptr_t heap_top;
    uintptr_t mapped_bytes;
    uintptr_t free_bytes;
    uint32_t total_allocs;
    uint32_t total_frees;
};

void kmalloc_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
struct kmalloc_stats kmalloc_get_stats(void);

#endif
