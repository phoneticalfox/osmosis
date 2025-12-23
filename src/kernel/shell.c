#include <stddef.h>
#include <stdint.h>

#include "osmosis/shell.h"
#include "osmosis/boot.h"
#include "osmosis/kprintf.h"
#include "osmosis/kmalloc.h"
#include "osmosis/pmm.h"
#include "osmosis/process.h"
#include "osmosis/tty.h"
#include "osmosis/vfs.h"
#include "osmosis/arch/i386/keyboard.h"
#include "osmosis/arch/i386/paging.h"
#include "osmosis/arch/i386/pit.h"

#define SHELL_PROMPT "osmosis> "
#define SHELL_MAX_INPUT 128

static const struct boot_info *boot_info_ref = NULL;

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
    tty_write("  help         - Show this help text\n");
    tty_write("  info         - Show kernel build and tick status\n");
    tty_write("  clear        - Clear the screen\n");
    tty_write("  memmap       - Show the bootloader-provided memory map\n");
    tty_write("  ticks        - Show PIT health snapshot\n");
    tty_write("  uptime       - Show PIT-tracked uptime\n");
    tty_write("  mem          - Show physical memory statistics\n");
    tty_write("  paging       - Show paging status\n");
    tty_write("  heap         - Show heap allocator statistics\n");
    tty_write("  alloc_test   - Allocate and free test blocks\n");
    tty_write("  sleep <ms>   - Pause for the requested milliseconds\n");
    tty_write("  ps           - List processes\n");
    tty_write("  ls           - List initramfs files\n");
    tty_write("  cat <path>   - Print an initramfs file\n");
}

static void shell_print_info(void) {
    uint32_t ticks = pit_ticks();
    uint32_t freq = pit_frequency();
    if (!freq) {
        freq = 100;
    }
    kprintf("OS/mosis kernel: v0.1 (ticks=%u @ %u Hz, free_frames=%u)\n",
            ticks, freq, pmm_free_frames());
    kprintf("Console: VGA text, IRQs enabled, PS/2 keyboard buffered, PIT %u Hz.\n", freq);
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

static void shell_print_paging(void) {
    struct paging_stats stats = paging_get_stats();
    kprintf("Paging: %s (CR3=0x%x)\n",
            stats.enabled ? "enabled" : "disabled",
            (uint32_t)stats.page_directory_phys);
    kprintf("Identity map: [0x%x, 0x%x) mapped_pages=%u tables=%u\n",
            (uint32_t)stats.identity_base,
            (uint32_t)stats.identity_limit,
            stats.mapped_pages,
            stats.page_table_count);
}

static void shell_print_heap(void) {
    struct kmalloc_stats stats = kmalloc_get_stats();
    kprintf("Heap: base=0x%x limit=0x%x top=0x%x mapped=%u bytes free_list=%u bytes\n",
            (uint32_t)stats.heap_base, (uint32_t)stats.heap_limit,
            (uint32_t)stats.heap_top, (uint32_t)stats.mapped_bytes,
            (uint32_t)stats.free_bytes);
    kprintf("Allocations: total=%u frees=%u\n",
            stats.total_allocs, stats.total_frees);
}

static void shell_alloc_test(void) {
    kprintf("Running heap allocation test...\n");
    void *a = kmalloc(64);
    void *b = kmalloc(128);
    void *c = kmalloc(32);

    if (!a || !b || !c) {
        kprintf("Allocation failure (a=%p b=%p c=%p)\n", a, b, c);
        return;
    }

    /* Touch memory to ensure mapping. */
    uint8_t *pa = (uint8_t *)a;
    uint8_t *pb = (uint8_t *)b;
    uint8_t *pc = (uint8_t *)c;
    for (int i = 0; i < 64; i++) {
        pa[i] = (uint8_t)i;
    }
    for (int i = 0; i < 128; i++) {
        pb[i] = (uint8_t)(i + 1);
    }
    for (int i = 0; i < 32; i++) {
        pc[i] = 0xAA;
    }

    /* Verify a small sample. */
    int ok = 1;
    for (int i = 0; i < 64; i++) {
        if (pa[i] != (uint8_t)i) {
            ok = 0;
            break;
        }
    }
    if (ok && pb[5] != 6) {
        ok = 0;
    }
    if (ok && pc[10] != 0xAA) {
        ok = 0;
    }

    kfree(a);
    kfree(b);
    kfree(c);

    kprintf("Heap test %s (a=%p b=%p c=%p)\n", ok ? "passed" : "failed", a, b, c);
}

static void shell_print_uptime(void) {
    uint32_t freq = pit_frequency();
    if (!freq) {
        freq = 100;
    }

    uint32_t ticks = pit_ticks();
    uint32_t seconds = ticks / freq;
    uint32_t ms_fraction = (ticks % freq) * 1000u / freq;

    kprintf("Uptime: %u.%03u seconds (%u ticks @ %u Hz)\n",
            seconds, ms_fraction, ticks, freq);
}

static void shell_prompt(void) {
    tty_write(SHELL_PROMPT);
}

static int parse_uint(const char *s, uint32_t *out_value) {
    if (!s || !*s || !out_value) {
        return 0;
    }

    uint32_t value = 0;
    while (*s) {
        if (*s < '0' || *s > '9') {
            return 0;
        }
        uint32_t next = value * 10u + (uint32_t)(*s - '0');
        if (next < value) {
            return 0; /* overflow */
        }
        value = next;
        s++;
    }

    *out_value = value;
    return 1;
}

static int match_command(const char *line, const char *cmd, const char **arg_out) {
    size_t i = 0;
    while (cmd[i]) {
        if (line[i] != cmd[i]) {
            return 0;
        }
        i++;
    }

    if (line[i] == '\0') {
        if (arg_out) {
            *arg_out = NULL;
        }
        return 1;
    }

    if (line[i] != ' ') {
        return 0;
    }

    while (line[i] == ' ') {
        i++;
    }

    if (arg_out) {
        *arg_out = &line[i];
    }
    return 1;
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
    } else if (str_eq(line, "uptime")) {
        shell_print_uptime();
    } else if (str_eq(line, "mem")) {
        shell_print_memory();
    } else if (str_eq(line, "memmap")) {
        boot_print_memory_map(boot_info_ref);
    } else if (str_eq(line, "paging")) {
        shell_print_paging();
    } else if (str_eq(line, "heap")) {
        shell_print_heap();
    } else if (str_eq(line, "alloc_test")) {
        shell_alloc_test();
    } else if (str_eq(line, "ps")) {
        process_list();
    } else if (str_eq(line, "ls")) {
        vfs_list();
    } else {
        const char *arg = NULL;
        if (match_command(line, "sleep", &arg) && arg && *arg) {
            uint32_t ms;
            if (parse_uint(arg, &ms)) {
                pit_sleep_ms(ms);
                kprintf("Slept for %u ms\n", ms);
            } else {
                kprintf("Invalid duration: %s\n", arg);
            }
        } else if (match_command(line, "cat", &arg) && arg && *arg) {
            const struct vfs_node *node = vfs_lookup(arg);
            if (!node) {
                kprintf("cat: not found: %s\n", arg);
            } else {
                char buf[128];
                uint32_t offset = 0;
                int read;
                while ((read = vfs_read(node, offset, buf, sizeof(buf))) > 0) {
                    for (int i = 0; i < read; i++) {
                        tty_putc(buf[i]);
                    }
                    offset += (uint32_t)read;
                }
                tty_putc('\n');
            }
        } else {
            kprintf("Unknown command: %s\n", line);
            shell_print_help();
        }
    }
}

void shell_init(const struct boot_info *boot) {
    boot_info_ref = boot;
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
