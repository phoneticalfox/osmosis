#ifndef OSMOSIS_ARCH_I386_PAGING_H
#define OSMOSIS_ARCH_I386_PAGING_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096u

#define PAGE_PRESENT 0x001u
#define PAGE_WRITE   0x002u
#define PAGE_USER    0x004u

struct boot_info;

struct paging_stats {
    int enabled;
    uintptr_t page_directory_phys;
    uintptr_t identity_base;
    uintptr_t identity_limit;
    uint32_t mapped_pages;
    uint32_t page_table_count;
};

void paging_init(const struct boot_info *boot);
int paging_map(uintptr_t virt, uintptr_t phys, uint32_t flags);
int paging_unmap(uintptr_t virt);
uintptr_t paging_resolve(uintptr_t virt);
int paging_enabled(void);
struct paging_stats paging_get_stats(void);
int paging_range_has_flags(uintptr_t virt, size_t len, uint32_t flags);

#endif
