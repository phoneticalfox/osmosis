#include <stddef.h>
#include <stdint.h>

#include "osmosis/arch/i386/paging.h"
#include "osmosis/boot.h"
#include "osmosis/kprintf.h"
#include "osmosis/panic.h"
#include "osmosis/pmm.h"

#define PAGE_TABLE_ENTRIES 1024u
#define PAGE_DIRECTORY_ENTRIES 1024u
#define PAGE_ALIGN_MASK (~(PAGE_SIZE - 1u))
#define IDENTITY_MAP_LIMIT (64u * 1024u * 1024u) /* 64 MiB cap for early identity */

struct page_table {
    uint32_t entries[PAGE_TABLE_ENTRIES];
} __attribute__((aligned(PAGE_SIZE)));

static uint32_t page_directory[PAGE_DIRECTORY_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uintptr_t identity_limit = 0;
static uint32_t mapped_pages = 0;
static uint32_t allocated_tables = 0;
static int paging_on = 0;

extern char _kernel_end[];

static inline uint32_t pd_index(uintptr_t addr) {
    return (uint32_t)((addr >> 22) & 0x3FFu);
}

static inline uint32_t pt_index(uintptr_t addr) {
    return (uint32_t)((addr >> 12) & 0x3FFu);
}

static inline void invlpg(uintptr_t addr) {
    __asm__ __volatile__("invlpg (%0)" :: "r"(addr) : "memory");
}

static inline uintptr_t align_up(uintptr_t value, uintptr_t align) {
    return (value + align - 1u) & ~(align - 1u);
}

static struct page_table *alloc_page_table(void) {
    uintptr_t frame = pmm_alloc_frame();
    if (!frame) {
        return NULL;
    }

    struct page_table *table = (struct page_table *)(frame);
    for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        table->entries[i] = 0;
    }
    allocated_tables++;
    return table;
}

static struct page_table *get_or_create_table(uintptr_t virt, uint32_t flags) {
    uint32_t index = pd_index(virt);
    uint32_t entry = page_directory[index];

    if (entry & PAGE_PRESENT) {
        uintptr_t table_addr = entry & PAGE_ALIGN_MASK;
        return (struct page_table *)table_addr;
    }

    struct page_table *table = alloc_page_table();
    if (!table) {
        return NULL;
    }

    page_directory[index] = ((uintptr_t)table & PAGE_ALIGN_MASK) | (flags & 0xFFFu) | PAGE_PRESENT;
    return table;
}

static int map_single(uintptr_t virt, uintptr_t phys, uint32_t flags) {
    if ((virt & (PAGE_SIZE - 1u)) || (phys & (PAGE_SIZE - 1u))) {
        return 0;
    }

    struct page_table *table = get_or_create_table(virt, flags | PAGE_WRITE);
    if (!table) {
        return 0;
    }

    uint32_t t_index = pt_index(virt);
    if (table->entries[t_index] & PAGE_PRESENT) {
        return 0;
    }

    table->entries[t_index] = (uint32_t)(phys & PAGE_ALIGN_MASK) | (flags & 0xFFFu) | PAGE_PRESENT;
    mapped_pages++;
    invlpg(virt);
    return 1;
}

static void identity_map_range(uintptr_t start, uintptr_t end) {
    uintptr_t aligned_start = start & PAGE_ALIGN_MASK;
    uintptr_t aligned_end = align_up(end, PAGE_SIZE);

    for (uintptr_t addr = aligned_start; addr < aligned_end; addr += PAGE_SIZE) {
        (void)map_single(addr, addr, PAGE_WRITE);
    }
}

static void load_page_directory(uintptr_t phys) {
    __asm__ __volatile__("mov %0, %%cr3" :: "r"(phys) : "memory");
}

static void enable_paging(void) {
    uint32_t cr0;
    __asm__ __volatile__("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000u; /* Paging enable bit */
    __asm__ __volatile__("mov %0, %%cr0" :: "r"(cr0) : "memory");
}

void paging_init(const struct boot_info *boot) {
    mapped_pages = 0;
    allocated_tables = 0;
    paging_on = 0;

    for (uint32_t i = 0; i < PAGE_DIRECTORY_ENTRIES; i++) {
        page_directory[i] = 0;
    }

    uintptr_t base_limit = align_up((uintptr_t)_kernel_end, PAGE_SIZE);
    if (base_limit < PAGE_SIZE * 4u) {
        base_limit = PAGE_SIZE * 4u; /* Keep first 16 KiB mapped for IVT/stack slack. */
    }

    uintptr_t max_identity = IDENTITY_MAP_LIMIT;
    if (boot && boot->mem_upper_kb) {
        uintptr_t reported_top = ((uintptr_t)boot->mem_upper_kb + 1024u) * 1024u;
        if (reported_top && reported_top < max_identity) {
            max_identity = reported_top;
        }
    }

    identity_limit = base_limit;
    if (identity_limit > max_identity) {
        identity_limit = max_identity;
    }

    identity_map_range(0, identity_limit);

    load_page_directory((uintptr_t)page_directory);
    enable_paging();
    paging_on = 1;

    kprintf("Paging: enabled, identity-mapped 0x%x bytes (%u pages, %u tables).\n",
            (uint32_t)identity_limit, mapped_pages, allocated_tables);
}

int paging_map(uintptr_t virt, uintptr_t phys, uint32_t flags) {
    int ok = map_single(virt, phys, flags);
    if (!ok) {
        return 0;
    }
    return 1;
}

int paging_unmap(uintptr_t virt) {
    if (virt & (PAGE_SIZE - 1u)) {
        return 0;
    }

    uint32_t d_index = pd_index(virt);
    uint32_t entry = page_directory[d_index];
    if (!(entry & PAGE_PRESENT)) {
        return 0;
    }

    struct page_table *table = (struct page_table *)(entry & PAGE_ALIGN_MASK);
    uint32_t t_index = pt_index(virt);
    if (!(table->entries[t_index] & PAGE_PRESENT)) {
        return 0;
    }

    table->entries[t_index] = 0;
    if (mapped_pages > 0) {
        mapped_pages--;
    }
    invlpg(virt);
    return 1;
}

uintptr_t paging_resolve(uintptr_t virt) {
    uint32_t d_index = pd_index(virt);
    uint32_t entry = page_directory[d_index];
    if (!(entry & PAGE_PRESENT)) {
        return 0;
    }

    struct page_table *table = (struct page_table *)(entry & PAGE_ALIGN_MASK);
    uint32_t t_index = pt_index(virt);
    uint32_t page_entry = table->entries[t_index];
    if (!(page_entry & PAGE_PRESENT)) {
        return 0;
    }

    return (page_entry & PAGE_ALIGN_MASK) | (virt & (PAGE_SIZE - 1u));
}

int paging_enabled(void) {
    uint32_t cr0;
    __asm__ __volatile__("mov %%cr0, %0" : "=r"(cr0));
    return (cr0 & 0x80000000u) != 0;
}

struct paging_stats paging_get_stats(void) {
    struct paging_stats stats;
    stats.enabled = paging_on && paging_enabled();
    stats.page_directory_phys = (uintptr_t)page_directory;
    stats.identity_base = 0;
    stats.identity_limit = identity_limit;
    stats.mapped_pages = mapped_pages;
    stats.page_table_count = allocated_tables;
    return stats;
}
