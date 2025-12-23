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
#include "osmosis/process.h"
#include "osmosis/vfs.h"

extern const uint8_t _binary_build_initramfs_bin_start[];
extern const uint8_t _binary_build_initramfs_bin_end[];

static void userland_finished(void) {
    kprintf("All user processes exited.\n");
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
    vfs_init(_binary_build_initramfs_bin_start,
             (uint32_t)(uintptr_t)(_binary_build_initramfs_bin_end - _binary_build_initramfs_bin_start));
    process_init();
    process_set_idle_callback(userland_finished);
    tss_init(KERNEL_BOOT_STACK_TOP);
    syscall_init();
    shell_init(boot);

    irq_enable();

    pit_wait_ticks(5);
    pit_health_poll();
    struct pit_health health = pit_health_latest();

    kprintf("Timer heartbeat detected (%d ticks, delta=%d, stalled=%d).\n",
            pit_ticks(), health.last_delta, health.stalled);

    int demo_pid = userland_bootstrap_demo();
    if (demo_pid < 0) {
        kprintf("Failed to start demo process.\n");
        userland_finished();
        return;
    }
    kprintf("User demo pid=%d staged; entering scheduler.\n", demo_pid);
    process_enter_first();
}
