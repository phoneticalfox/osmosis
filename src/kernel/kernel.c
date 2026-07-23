#include <stdint.h>

#include "osmosis/arch/i386/idt.h"
#include "osmosis/arch/i386/irq.h"
#include "osmosis/arch/i386/multiboot.h"
#include "osmosis/arch/i386/pit.h"
#include "osmosis/arch/i386/keyboard.h"
#include "osmosis/arch/i386/serial.h"
#include "osmosis/arch/i386/qemu.h"
#include "osmosis/arch/i386/segments.h"
#include "osmosis/arch/i386/syscall.h"
#include "osmosis/arch/i386/tss.h"
#include "osmosis/boot.h"
#include "osmosis/kprintf.h"
#include "osmosis/panic.h"
#include "osmosis/arch/i386/paging.h"
#include "osmosis/pmm.h"
#include "osmosis/kmalloc.h"
#include "osmosis/tty.h"
#include "osmosis/shell.h"
#include "osmosis/userland.h"
#include "osmosis/vfs.h"

extern const uint8_t __initramfs_start[];
extern const uint8_t __initramfs_end[];

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
    boot_print_memory_map(boot);

    kprintf("IRQ routing: PIC remapped to %d-%d\n", IRQ_BASE, IRQ_MAX);
    kprintf("Keyboard: PS/2 set 1 (IRQ1)\n");
    pmm_init(boot);
    paging_init(boot);
    kmalloc_init();
    tss_init();
    syscall_init();
    shell_init(boot);

    const uint8_t *initramfs = __initramfs_start;
    uint32_t initramfs_size =
        (uint32_t)(__initramfs_end - __initramfs_start);
    vfs_init(initramfs, initramfs_size);

    irq_enable();

    pit_wait_ticks(5);
    pit_health_poll();
    struct pit_health health = pit_health_latest();

    kprintf("Timer heartbeat detected (%d ticks, delta=%d, stalled=%d).\n",
            pit_ticks(), health.last_delta, health.stalled);
    int user_exit = userland_run_demo();
    kprintf("User mode demo completed (exit=%d).\n", user_exit);
    kprintf("\n\"Correctness First, Clarity Always.\"\n");

#ifdef CONFIG_QEMU_EXIT
    kprintf("Boot verification %s; exiting via QEMU debug port.\n",
            user_exit == 0 ? "passed" : "failed");
    /* isa-debug-exit reports (value << 1) | 1: success is status 33. */
    qemu_exit(user_exit == 0 ? 0x10u : 0x11u);
#else
    kprintf("Kernel shell ready. Type 'help' for commands.\n");
    shell_run();
#endif

    for (;;) {
        __asm__ __volatile__("hlt");
    }
}
