#include "osmosis/arch/i386/pit.h"
#include "osmosis/arch/i386/io.h"
#include "osmosis/arch/i386/irq.h"

#define PIT_INPUT_HZ 1193182
#define PIT_COMMAND  0x43
#define PIT_CHANNEL0 0x40

static volatile uint32_t pit_tick_count = 0;

static void pit_irq_handler(struct isr_frame *frame) {
    (void)frame;
    pit_tick_count++;
}

void pit_init(uint32_t frequency_hz) {
    if (frequency_hz == 0) {
        frequency_hz = 100; /* Default to 100 Hz when unspecified. */
    }

    uint32_t divisor = PIT_INPUT_HZ / frequency_hz;
    if (divisor == 0) {
        divisor = 1; /* Clamp so the PIT always receives a valid divisor. */
    }

    /* Channel 0, lobyte/hibyte access, mode 3 (square wave), binary. */
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

    irq_install_handler(0, pit_irq_handler);
}

uint32_t pit_ticks(void) {
    return pit_tick_count;
}
