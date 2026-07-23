#include <stdint.h>

#include "osmosis/boot.h"
#include "osmosis/kprintf.h"
#include "osmosis/panic.h"

static struct boot_info boot_state;

static void boot_store_basic(const struct multiboot_info *mb_info) {
    if (mb_info->flags & MULTIBOOT_INFO_MEMORY) {
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
        uint32_t remaining = mmap_end - offset;
        if (remaining < sizeof(uint32_t)) {
            panic("Malformed multiboot memory map header");
        }

        const struct multiboot_mmap_entry *entry = (const struct multiboot_mmap_entry *)(cursor + offset);
        const uint32_t minimum_payload = sizeof(*entry) - sizeof(entry->size);
        if (entry->size < minimum_payload || entry->size > remaining - sizeof(entry->size)) {
            panic("Malformed multiboot memory map entry");
        }

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

const char *boot_memory_type_name(uint32_t type) {
    switch (type) {
        case BOOT_MEMORY_USABLE:
            return "usable";
        case BOOT_MEMORY_RESERVED:
            return "reserved";
        default:
            return "other";
    }
}

static void boot_print_hex64(uint64_t value) {
    uint32_t high = (uint32_t)(value >> 32);
    uint32_t low = (uint32_t)value;
    if (high) {
        kprintf("0x%x%08x", high, low);
    } else {
        kprintf("0x%x", low);
    }
}

void boot_print_memory_map(const struct boot_info *boot) {
    if (!boot) {
        kprintf("Memory map: unavailable (no boot info)\n");
        return;
    }

    kprintf("Memory map (%u entr%s):\n",
            boot->region_count,
            boot->region_count == 1 ? "y" : "ies");

    for (uint32_t i = 0; i < boot->region_count; i++) {
        const struct boot_memory_region *r = &boot->regions[i];
        kprintf("  [%u] base=", i);
        boot_print_hex64(r->base);
        kprintf(" length=");
        boot_print_hex64(r->length);
        kprintf(" bytes type=%s (%u)\n", boot_memory_type_name(r->type), r->type);
    }
}
