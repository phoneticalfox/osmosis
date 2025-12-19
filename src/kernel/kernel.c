#include <stdint.h>

#include "osmosis/tty.h"
#include "osmosis/kprintf.h"
#include "osmosis/panic.h"
#include "osmosis/arch/i386/idt.h"
#include "osmosis/arch/i386/irq.h"
#include "osmosis/arch/i386/pit.h"
#include "osmosis/arch/i386/keyboard.h"
#include "osmosis/arch/i386/serial.h"
#include "osmosis/arch/i386/qemu.h"
#include "osmosis/shell.h"

void kernel_main(void) {
    serial_init();
    tty_init();
    idt_init();
    irq_init();
    pit_init(100);
    keyboard_init();

    tty_set_color(COLOR_LIGHT_GREEN, COLOR_BLACK);
    kprintf("Welcome to OS/mosis.\n");
    kprintf("System Integrity: Online\n\n");

    kprintf("Kernel Version: 0.1\n");
    kprintf("Mode: 32-bit Protected\n");
    kprintf("VGA Buffer: 0x%x\n", VGA_MEMORY);

    kprintf("IRQ routing: PIC remapped to %d-%d\n", IRQ_BASE, IRQ_MAX);
    kprintf("Keyboard: PS/2 set 1 (IRQ1)\n");

    irq_enable();

    uint32_t target_ticks = pit_ticks() + 5;
    while (pit_ticks() < target_ticks) {
        __asm__ __volatile__("hlt");
    }

    kprintf("Timer heartbeat detected (%d ticks).\n", target_ticks);
    kprintf("\n\"Correctness First, Clarity Always.\"\n");

#ifdef CONFIG_QEMU_EXIT
    kprintf("Exiting via QEMU debug port.\n");
    qemu_exit(0);
#else
    kprintf("Keyboard buffer armed. Starting kernel shell.\n");
    shell_run();
#endif

    for (;;) {
        __asm__ __volatile__("hlt");
    }
}
