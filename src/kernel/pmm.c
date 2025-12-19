#include <stddef.h>
#include <stdint.h>

#include "osmosis/boot.h"
#include "osmosis/kprintf.h"
#include "osmosis/panic.h"
#include "osmosis/pmm.h"

#define FRAME_SIZE 4096
#define PMM_MAX_FRAMES (1024 * 1024) /* 4 GiB / 4 KiB frames */

static uint8_t frame_bitmap[PMM_MAX_FRAMES / 8];
static uint32_t frame_count;
static uint32_t free_frame_count;

extern char _kernel_start[];
extern char _kernel_end[];

static inline void frame_set(uint32_t frame) {
    frame_bitmap[frame / 8] |= (uint8_t)(1 << (frame % 8));
}

static inline void frame_clear(uint32_t frame) {
    frame_bitmap[frame / 8] &= (uint8_t)~(1 << (frame % 8));
}

static inline int frame_test(uint32_t frame) {
    return frame_bitmap[frame / 8] & (uint8_t)(1 << (frame % 8));
}

static void mark_range(uint64_t base, uint64_t length, int free) {
    uint64_t start_frame = base / FRAME_SIZE;
    uint64_t end_frame = (base + length + FRAME_SIZE - 1) / FRAME_SIZE;

    if (end_frame > frame_count) {
        end_frame = frame_count;
    }

    for (uint64_t f = start_frame; f < end_frame; f++) {
        if (free) {
            if (frame_test((uint32_t)f)) {
                frame_clear((uint32_t)f);
                free_frame_count++;
            }
        } else {
            if (!frame_test((uint32_t)f)) {
                free_frame_count--;
                frame_set((uint32_t)f);
            }
        }
    }
}

static uint64_t max_usable_address(const struct boot_info *boot) {
    uint64_t max_addr = 0;
    for (uint32_t i = 0; i < boot->region_count; i++) {
        const struct boot_memory_region *region = &boot->regions[i];
        if (region->type != BOOT_MEMORY_USABLE) {
            continue;
        }
        uint64_t end = region->base + region->length;
        if (end > max_addr) {
            max_addr = end;
        }
    }
    return max_addr;
}

static void reserve_kernel(const struct boot_info *boot) {
    (void)boot;
    uintptr_t kernel_start = (uintptr_t)_kernel_start;
    uintptr_t kernel_end = (uintptr_t)_kernel_end;

    uintptr_t aligned_start = kernel_start & ~(FRAME_SIZE - 1);
    uintptr_t aligned_end = (kernel_end + FRAME_SIZE - 1) & ~(FRAME_SIZE - 1);

    mark_range(aligned_start, aligned_end - aligned_start, 0);
}

static void reserve_bootloader_payload(const struct boot_info *boot) {
    if (!boot->multiboot_ptr) {
        return;
    }

    uintptr_t addr = (uintptr_t)boot->multiboot_ptr;
    mark_range(addr, FRAME_SIZE, 0);
}

void pmm_init(const struct boot_info *boot) {
    if (!boot) {
        panic("PMM init requires boot info");
    }

    uint64_t highest_address = max_usable_address(boot);
    frame_count = (uint32_t)((highest_address + FRAME_SIZE - 1) / FRAME_SIZE);

    if (frame_count > PMM_MAX_FRAMES) {
        frame_count = PMM_MAX_FRAMES;
    }

    for (uint32_t i = 0; i < PMM_MAX_FRAMES / 8; i++) {
        frame_bitmap[i] = 0xFF;
    }

    free_frame_count = 0;

    for (uint32_t i = 0; i < boot->region_count; i++) {
        const struct boot_memory_region *region = &boot->regions[i];
        if (region->type != BOOT_MEMORY_USABLE) {
            continue;
        }
        mark_range(region->base, region->length, 1);
    }

    /* Keep low memory and the kernel image reserved. */
    mark_range(0, 0x100000, 0);
    reserve_kernel(boot);
    reserve_bootloader_payload(boot);

    kprintf("PMM: %d frames (%d KiB) detected, %d frames free.\n",
            frame_count, (frame_count * FRAME_SIZE) / 1024, free_frame_count);
}

uintptr_t pmm_alloc_frame(void) {
    for (uint32_t frame = 0; frame < frame_count; frame++) {
        if (!frame_test(frame)) {
            frame_set(frame);
            if (free_frame_count > 0) {
                free_frame_count--;
            }
            return (uintptr_t)frame * FRAME_SIZE;
        }
    }

    return 0;
}

void pmm_free_frame(uintptr_t addr) {
    uint32_t frame = (uint32_t)(addr / FRAME_SIZE);
    if (frame >= frame_count) {
        return;
    }

    if (frame_test(frame)) {
        frame_clear(frame);
        free_frame_count++;
    }
}

uint32_t pmm_total_frames(void) {
    return frame_count;
}

uint32_t pmm_free_frames(void) {
    return free_frame_count;
}
