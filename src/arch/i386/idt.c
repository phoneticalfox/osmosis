#include "osmosis/arch/i386/idt.h"
#include "osmosis/arch/i386/isr.h"

#define IDT_ENTRIES 256
#define KERNEL_CODE_SELECTOR 0x08
#define IDT_FLAG_PRESENT 0x80
#define IDT_FLAG_INT_GATE_32 0x0E
#define IDT_FLAG_RING0 0x00

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtp;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

static void idt_flush(void) {
    __asm__ __volatile__("lidt %0" : : "m"(idtp));
}

static void idt_install_exceptions(void) {
    uint8_t flags = IDT_FLAG_PRESENT | IDT_FLAG_INT_GATE_32 | IDT_FLAG_RING0;

    void (*exceptions[])(void) = {
        isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7,
        isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15,
        isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
        isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31,
    };

    for (uint8_t i = 0; i < sizeof(exceptions) / sizeof(exceptions[0]); i++) {
        idt_set_gate(i, (uint32_t)(uintptr_t)exceptions[i], KERNEL_CODE_SELECTOR, flags);
    }
}

void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (uint32_t)(uintptr_t)&idt;

    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    idt_install_exceptions();
    idt_flush();
}
