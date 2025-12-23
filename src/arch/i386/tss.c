#include "osmosis/arch/i386/tss.h"

#include "osmosis/arch/i386/segments.h"
#include "osmosis/kprintf.h"

static struct tss_entry tss;

static uint64_t gdt_make_descriptor(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    uint64_t descriptor = 0;
    descriptor |= (uint64_t)(limit & 0xFFFFu);
    descriptor |= (uint64_t)(base & 0xFFFFFFu) << 16;
    descriptor |= (uint64_t)access << 40;
    descriptor |= (uint64_t)((limit >> 16) & 0x0Fu) << 48;
    descriptor |= (uint64_t)(flags & 0xF0u) << 48;
    descriptor |= (uint64_t)((base >> 24) & 0xFFu) << 56;
    return descriptor;
}

void tss_set_kernel_stack(uint32_t kernel_stack_top) {
    tss.esp0 = kernel_stack_top;
}

void tss_init(uint32_t kernel_stack_top) {
    extern struct gdt_ptr GDTR;
    extern uint64_t GDT_START[];

    for (unsigned int i = 0; i < sizeof(struct tss_entry) / sizeof(uint32_t); i++) {
        ((uint32_t *)&tss)[i] = 0;
    }

    tss_set_kernel_stack(kernel_stack_top);
    tss.ss0 = KERNEL_DATA_SELECTOR;
    tss.cs = USER_CODE_SELECTOR | 0x03;
    tss.ds = USER_DATA_SELECTOR | 0x03;
    tss.es = USER_DATA_SELECTOR | 0x03;
    tss.fs = USER_DATA_SELECTOR | 0x03;
    tss.gs = USER_DATA_SELECTOR | 0x03;
    tss.iomap_base = sizeof(struct tss_entry);

    /* Update the TSS descriptor (entry 5) in the preloaded GDT. */
    const uint32_t limit = sizeof(struct tss_entry) - 1;
    uint64_t descriptor = gdt_make_descriptor((uint32_t)(uintptr_t)&tss, limit, 0x89, 0x00);
    GDT_START[5] = descriptor;

    /* Reload the GDT to pick up the TSS descriptor update. */
    __asm__ __volatile__("lgdt %0" : : "m"(GDTR));

    /* Load the task register with the TSS selector. */
    __asm__ __volatile__("ltr %0" : : "r"(TSS_SELECTOR));

    kprintf("TSS: esp0=0x%x ss0=0x%x selector=0x%x\n", tss.esp0, tss.ss0, TSS_SELECTOR);
}
