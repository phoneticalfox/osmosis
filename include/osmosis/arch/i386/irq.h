#ifndef OSMOSIS_ARCH_I386_IRQ_H
#define OSMOSIS_ARCH_I386_IRQ_H

#include <stdint.h>
#include "osmosis/arch/i386/isr.h"

#define IRQ_BASE 32
#define IRQ_MAX  47

typedef void (*irq_handler_t)(struct isr_frame *frame);

void irq_init(void);
void irq_install_handler(uint8_t irq, irq_handler_t handler);
void irq_clear_handler(uint8_t irq);
void irq_enable(void);

void irq_handler(struct isr_frame *frame);

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

#endif
