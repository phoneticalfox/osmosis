#include "osmosis/tty.h"
#include "osmosis/kprintf.h"
#include "osmosis/panic.h"
#include "osmosis/arch/i386/idt.h"

void kernel_main(void) {
    tty_init();
    idt_init();

    tty_set_color(COLOR_LIGHT_GREEN, COLOR_BLACK);
    kprintf("Welcome to OS/mosis.\n");
    kprintf("System Integrity: Online\n\n");

    kprintf("Kernel Version: 0.1\n");
    kprintf("Mode: 32-bit Protected\n");
    kprintf("VGA Buffer: 0x%x\n", VGA_MEMORY);

    kprintf("\n\"Correctness First, Clarity Always.\"\n");

    for (;;) {
        __asm__ __volatile__("hlt");
    }
}
