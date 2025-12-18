#include "osmosis/arch/i386/irq.h"
#include "osmosis/arch/i386/idt.h"
#include "osmosis/arch/i386/io.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define PIC_EOI 0x20

static irq_handler_t irq_handlers[16] = {0};

static void pic_remap(void) {
    /* Start initialization sequence (cascade mode). */
    outb(PIC1_COMMAND, 0x11);
    io_wait();
    outb(PIC2_COMMAND, 0x11);
    io_wait();

    /* Remap offsets so exceptions (0-31) remain untouched. */
    outb(PIC1_DATA, IRQ_BASE);
    io_wait();
    outb(PIC2_DATA, IRQ_BASE + 8);
    io_wait();

    /* Tell Master PIC about Slave at IRQ2 (0000 0100). */
    outb(PIC1_DATA, 0x04);
    io_wait();
    /* Tell Slave PIC its cascade identity (0000 0010). */
    outb(PIC2_DATA, 0x02);
    io_wait();

    /* Set both PICs to 8086/88 mode. */
    outb(PIC1_DATA, 0x01);
    io_wait();
    outb(PIC2_DATA, 0x01);
    io_wait();

    /* Clear masks. */
    outb(PIC1_DATA, 0x0);
    outb(PIC2_DATA, 0x0);
}

void irq_install_handler(uint8_t irq, irq_handler_t handler) {
    if (irq < 16) {
        irq_handlers[irq] = handler;
    }
}

void irq_clear_handler(uint8_t irq) {
    if (irq < 16) {
        irq_handlers[irq] = 0;
    }
}

void irq_init(void) {
    uint8_t flags = 0x80 | 0x0E | 0x00; /* present | 32-bit gate | ring0 */

    pic_remap();

    void (*irqs[])(void) = {
        irq0,  irq1,  irq2,  irq3,  irq4,  irq5,  irq6,  irq7,
        irq8,  irq9,  irq10, irq11, irq12, irq13, irq14, irq15,
    };

    for (uint8_t i = 0; i < sizeof(irqs) / sizeof(irqs[0]); i++) {
        idt_set_gate(IRQ_BASE + i, (uint32_t)(uintptr_t)irqs[i], 0x08, flags);
    }
}

void irq_enable(void) {
    __asm__ __volatile__("sti");
}

void irq_disable(void) {
    __asm__ __volatile__("cli" ::: "memory");
}

uint32_t irq_save(void) {
    uint32_t flags;
    __asm__ __volatile__("pushfl; popl %0; cli" : "=r"(flags) :: "memory");
    return flags;
}

void irq_restore(uint32_t flags) {
    __asm__ __volatile__("pushl %0; popfl" :: "r"(flags) : "memory", "cc");
}

void irq_handler(struct isr_frame *frame) {
    if (frame->int_no >= IRQ_BASE && frame->int_no <= IRQ_MAX) {
        uint8_t irq_no = (uint8_t)(frame->int_no - IRQ_BASE);
        irq_handler_t handler = irq_handlers[irq_no];
        if (handler) {
            handler(frame);
        }

        if (frame->int_no >= IRQ_BASE + 8) {
            outb(PIC2_COMMAND, PIC_EOI);
        }
        outb(PIC1_COMMAND, PIC_EOI);
    }
}
