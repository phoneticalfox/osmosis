#include <stddef.h>
#include <stdint.h>

#include "osmosis/shell.h"
#include "osmosis/kprintf.h"
#include "osmosis/pmm.h"
#include "osmosis/tty.h"
#include "osmosis/arch/i386/keyboard.h"
#include "osmosis/arch/i386/pit.h"

#define SHELL_PROMPT "osmosis> "
#define SHELL_MAX_INPUT 128

static int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == *b;
}

static void shell_print_help(void) {
    tty_write("Commands:\n");
    tty_write("  help   - Show this help text\n");
    tty_write("  info   - Show kernel build and tick status\n");
    tty_write("  clear  - Clear the screen\n");
    tty_write("  ticks  - Show PIT health snapshot\n");
    tty_write("  mem    - Show physical memory statistics\n");
}

static void shell_print_info(void) {
    uint32_t ticks = pit_ticks();
    kprintf("OS/mosis kernel: v0.1 (ticks=%u, free_frames=%u)\n", ticks, pmm_free_frames());
    kprintf("Console: VGA text, IRQs enabled, PS/2 keyboard buffered.\n");
}

static void shell_print_ticks(void) {
    pit_health_poll();
    struct pit_health health = pit_health_latest();
    kprintf("PIT ticks: %u (last delta=%u stalled=%d)\n",
            pit_ticks(), health.last_delta, health.stalled);
}

static void shell_print_memory(void) {
    uint32_t total_frames = pmm_total_frames();
    uint32_t free_frames = pmm_free_frames();
    kprintf("Physical memory: total=%u KiB free=%u KiB (%u/%u frames free)\n",
            (total_frames * 4), (free_frames * 4), free_frames, total_frames);
}

static void shell_prompt(void) {
    tty_write(SHELL_PROMPT);
}

static void shell_handle_line(const char *line) {
    if (!line || !*line) {
        return;
    }

    if (str_eq(line, "help")) {
        shell_print_help();
    } else if (str_eq(line, "info")) {
        shell_print_info();
    } else if (str_eq(line, "clear")) {
        tty_clear();
    } else if (str_eq(line, "ticks")) {
        shell_print_ticks();
    } else if (str_eq(line, "mem")) {
        shell_print_memory();
    } else {
        kprintf("Unknown command: %s\n", line);
        shell_print_help();
    }
}

void shell_run(void) {
    char buffer[SHELL_MAX_INPUT];
    size_t len = 0;

    tty_write("\nEntering kernel shell (minimal, buffered input).\n");
    tty_write("Type 'help' to see available commands.\n");
    shell_prompt();

    for (;;) {
        char c;
        if (!keyboard_buffer_read(&c)) {
            __asm__ __volatile__("hlt");
            continue;
        }

        if (c == '\r') {
            continue; /* Ignore carriage return, wait for newline. */
        }

        if (c == '\b') {
            if (len > 0) {
                len--;
                tty_write("\b \b");
            }
            continue;
        }

        if (c == '\n') {
            tty_putc('\n');
            buffer[len] = '\0';
            shell_handle_line(buffer);
            len = 0;
            shell_prompt();
            continue;
        }

        if (len + 1 < SHELL_MAX_INPUT) {
            buffer[len++] = c;
            tty_putc(c);
        }
    }
}
