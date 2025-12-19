#include <stddef.h>
#include <stdint.h>

#include "osmosis/shell.h"
#include "osmosis/tty.h"
#include "osmosis/kprintf.h"
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
}

static void shell_print_info(void) {
    uint32_t ticks = pit_ticks();
    kprintf("OS/mosis kernel: v0.1 (ticks=%u)\n", ticks);
    kprintf("Console: VGA text, IRQs enabled, PS/2 keyboard buffered\n");
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
