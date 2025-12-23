#ifndef OSMOSIS_BOOT_H
#define OSMOSIS_BOOT_H

#include <stdint.h>

#include "osmosis/arch/i386/multiboot.h"

#define BOOT_MAX_MEMORY_REGIONS 32

enum boot_memory_type {
    BOOT_MEMORY_USABLE = 1,
    BOOT_MEMORY_RESERVED = 2
};

struct boot_memory_region {
    uint64_t base;
    uint64_t length;
    uint32_t type;
};

struct boot_info {
    uint32_t multiboot_magic;
    uint32_t multiboot_flags;
    const struct multiboot_info *multiboot_ptr;
    uint32_t mem_lower_kb;
    uint32_t mem_upper_kb;
    uint32_t region_count;
    struct boot_memory_region regions[BOOT_MAX_MEMORY_REGIONS];
};

const struct boot_info *boot_info_init(uint32_t magic, const struct multiboot_info *mb_info);
const char *boot_memory_type_name(uint32_t type);
void boot_print_memory_map(const struct boot_info *boot);

#endif
