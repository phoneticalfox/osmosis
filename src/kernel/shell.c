#include <stddef.h>
#include <stdint.h>

#include "osmosis/shell.h"
#include "osmosis/tty.h"
#include "osmosis/kprintf.h"
#include "osmosis/arch/i386/keyboard.h"
#include "osmosis/arch/i386/pit.h"

#define SHELL_PROMPT "osmosis> "
#define SHELL_MAX_INPUT 128
/* Must match the PIT frequency configured during init (pit_init(100)). */
#define SHELL_TICK_RATE_HZ 100

typedef struct {
    const char *name;
    const char *help;
    void (*handler)(const char *args);
} shell_command_t;

static int str_starts_with(const char *str, const char *prefix) {
    while (*prefix && *str) {
        if (*str != *prefix) {
            return 0;
        }
        str++;
        prefix++;
    }
    return *prefix == '\0';
}

static void cmd_help(const char *args);
static void cmd_info(const char *args);
static void cmd_clear(const char *args);
static void cmd_echo(const char *args);
static void cmd_uptime(const char *args);

static const shell_command_t commands[] = {
    { "help",  "Show this help text", cmd_help },
    { "info",  "Show kernel build and tick status", cmd_info },
    { "clear", "Clear the screen", cmd_clear },
    { "echo",  "Echo text back to the console", cmd_echo },
    { "uptime","Print tick count and approximate seconds", cmd_uptime },
};

static void shell_print_help(void) {
    tty_write("Commands:\n");
    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        kprintf("  %-5s - %s\n", commands[i].name, commands[i].help);
    }
}

static void cmd_help(const char *args) {
    (void)args;
    shell_print_help();
}

static void cmd_info(const char *args) {
    (void)args;
    uint32_t ticks = pit_ticks();
    kprintf("OS/mosis kernel: v0.1 (ticks=%u)\n", ticks);
    kprintf("Console: VGA text, IRQs enabled, PS/2 keyboard buffered\n");
}

static void cmd_clear(const char *args) {
    (void)args;
    tty_clear();
}

static void cmd_echo(const char *args) {
    if (!args) {
        tty_putc('\n');
        return;
    }

    while (*args == ' ') {
        args++;
    }

    tty_write(args);
    tty_putc('\n');
}

static void cmd_uptime(const char *args) {
    (void)args;
    uint32_t ticks = pit_ticks();
    uint32_t seconds = ticks / SHELL_TICK_RATE_HZ;
    uint32_t remainder = ticks % SHELL_TICK_RATE_HZ;
    kprintf("Uptime: %u ticks (~%u.%02us at %u Hz)\n",
            ticks, seconds, (remainder * 100) / SHELL_TICK_RATE_HZ,
            SHELL_TICK_RATE_HZ);
}

static void shell_prompt(void) {
    tty_write(SHELL_PROMPT);
}

static void shell_handle_line(const char *line) {
    if (!line || !*line) {
        return;
    }

    const shell_command_t *matched = 0;
    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        const char *name = commands[i].name;
        size_t idx = 0;
        while (line[idx] && line[idx] != ' ') {
            idx++;
        }

        size_t name_len = 0;
        while (name[name_len]) {
            name_len++;
        }

        if (idx == name_len && str_starts_with(line, name)) {
            matched = &commands[i];
            break;
        }
    }

    if (!matched) {
        kprintf("Unknown command: %s\n", line);
        shell_print_help();
        return;
    }

    const char *args = line;
    while (*args && *args != ' ') {
        args++;
    }
    if (*args == ' ') {
        args++;
    }

    matched->handler(args);
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
