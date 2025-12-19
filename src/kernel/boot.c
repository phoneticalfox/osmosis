#include <stdint.h>

#include "osmosis/boot.h"
#include "osmosis/panic.h"

static struct boot_info boot_state;

static void boot_store_basic(const struct multiboot_info *mb_info) {
    if (mb_info->flags & 0x1) {
        boot_state.mem_lower_kb = mb_info->mem_lower;
        boot_state.mem_upper_kb = mb_info->mem_upper;
    }
}

static void boot_store_mmap(const struct multiboot_info *mb_info) {
    if (!(mb_info->flags & MULTIBOOT_INFO_MEM_MAP)) {
        boot_state.region_count = 0;
        return;
    }

    uint32_t offset = 0;
    uint32_t mmap_end = mb_info->mmap_length;
    const uint8_t *cursor = (const uint8_t *)(uintptr_t)mb_info->mmap_addr;

    boot_state.region_count = 0;

    while (offset < mmap_end && boot_state.region_count < BOOT_MAX_MEMORY_REGIONS) {
        const struct multiboot_mmap_entry *entry = (const struct multiboot_mmap_entry *)(cursor + offset);
        boot_state.regions[boot_state.region_count].base = entry->addr;
        boot_state.regions[boot_state.region_count].length = entry->len;
        boot_state.regions[boot_state.region_count].type = entry->type;
        boot_state.region_count++;

        offset += entry->size + sizeof(entry->size);
    }
}

const struct boot_info *boot_info_init(uint32_t magic, const struct multiboot_info *mb_info) {
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        panic("Invalid multiboot magic");
    }

    if (!mb_info) {
        panic("Missing multiboot info");
    }

    boot_state.multiboot_magic = magic;
    boot_state.multiboot_flags = mb_info->flags;
    boot_state.multiboot_ptr = mb_info;
    boot_state.mem_lower_kb = 0;
    boot_state.mem_upper_kb = 0;
    boot_state.region_count = 0;

    boot_store_basic(mb_info);
    boot_store_mmap(mb_info);

    return &boot_state;
}
