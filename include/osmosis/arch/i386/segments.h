#ifndef OSMOSIS_ARCH_I386_SEGMENTS_H
#define OSMOSIS_ARCH_I386_SEGMENTS_H

#include <stdint.h>

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define USER_CODE_SELECTOR   0x18
#define USER_DATA_SELECTOR   0x20
#define TSS_SELECTOR         0x28

#define KERNEL_BOOT_STACK_TOP 0x90000u

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

extern struct gdt_ptr GDTR;
extern uint64_t GDT_START[];

#endif
