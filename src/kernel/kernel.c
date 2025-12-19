#include <stdint.h>

#include "osmosis/arch/i386/idt.h"
#include "osmosis/arch/i386/irq.h"
#include "osmosis/arch/i386/multiboot.h"
#include "osmosis/arch/i386/pit.h"
#include "osmosis/arch/i386/keyboard.h"
#include "osmosis/arch/i386/serial.h"
#include "osmosis/arch/i386/qemu.h"
#include "osmosis/boot.h"
#include "osmosis/kprintf.h"
#include "osmosis/panic.h"
#include "osmosis/pmm.h"
#include "osmosis/tty.h"
#include "osmosis/shell.h"

static void print_memory_map(const struct boot_info *boot) {
    kprintf("Memory map (%d entr%s):\n", boot->region_count, boot->region_count == 1 ? "y" : "ies");
    for (uint32_t i = 0; i < boot->region_count; i++) {
        const struct boot_memory_region *r = &boot->regions[i];
        uint32_t base_mb = (uint32_t)(r->base / (1024 * 1024));
        uint32_t len_mb = (uint32_t)(r->length / (1024 * 1024));
        kprintf("  [%d] base=%d MB len=%d MB type=%d\n", i, base_mb, len_mb, r->type);
    }
}

void kernel_main(uint32_t mb_magic, uint32_t mb_info_addr) {
    serial_init();
    tty_init();
    const struct multiboot_info *mb_info = (const struct multiboot_info *)(uintptr_t)mb_info_addr;
    const struct boot_info *boot = boot_info_init(mb_magic, mb_info);
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
    print_memory_map(boot);

    kprintf("IRQ routing: PIC remapped to %d-%d\n", IRQ_BASE, IRQ_MAX);
    kprintf("Keyboard: PS/2 set 1 (IRQ1)\n");
    pmm_init(boot);

    irq_enable();

    pit_wait_ticks(5);
    pit_health_poll();
    struct pit_health health = pit_health_latest();

    kprintf("Timer heartbeat detected (%d ticks, delta=%d, stalled=%d).\n",
            pit_ticks(), health.last_delta, health.stalled);
    kprintf("\n\"Correctness First, Clarity Always.\"\n");

#ifdef CONFIG_QEMU_EXIT
    kprintf("Exiting via QEMU debug port.\n");
    qemu_exit(0);
#else
    kprintf("Kernel shell ready. Type 'help' for commands.\n");
    shell_run();
#endif

    for (;;) {
        __asm__ __volatile__("hlt");
    }
}
