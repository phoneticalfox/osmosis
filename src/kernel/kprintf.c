#include "osmosis/kprintf.h"
#include "osmosis/tty.h"
#include "osmosis/arch/i386/serial.h"

#include <stdarg.h>
#include <stdint.h>

static void out_char(char c) {
    tty_putc(c);
    serial_write_char(c);
}

static void out_str(const char *s) {
    if (!s) {
        return;
    }
    while (*s) {
        out_char(*s++);
    }
}

static void print_hex(uint32_t value, int width, char pad) {
    const char *hex_digits = "0123456789ABCDEF";
    char buffer[8];
    int idx = 0;

    do {
        buffer[idx++] = hex_digits[value & 0xF];
        value >>= 4;
    } while (value && idx < 8);

    while (idx < width && idx < 8) {
        buffer[idx++] = pad;
    }

    for (int i = idx - 1; i >= 0; i--) {
        out_char(buffer[i]);
    }
}

static void print_uint(uint32_t value, int width, char pad) {
    char buffer[12];
    int idx = 0;

    do {
        buffer[idx++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0 && idx < 11);

    while (idx < width && idx < 11) {
        buffer[idx++] = pad;
    }

    for (int i = idx - 1; i >= 0; i--) {
        out_char(buffer[i]);
    }
}

static void print_int(int value, int width, char pad) {
    char buffer[12];
    int idx = 0;
    int is_negative = value < 0;
    uint32_t magnitude = is_negative ? (uint32_t)(-(value + 1)) + 1 : (uint32_t)value;

    do {
        buffer[idx++] = '0' + (magnitude % 10);
        magnitude /= 10;
    } while (magnitude > 0 && idx < 11);

    int total_len = idx + (is_negative ? 1 : 0);
    if (width < total_len) {
        width = total_len;
    }

    int pad_count = width - total_len;
    if (is_negative) {
        out_char('-');
    }

    for (int i = 0; i < pad_count; i++) {
        out_char(pad);
    }

    for (int i = idx - 1; i >= 0; i--) {
        out_char(buffer[i]);
    }
}

int kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (const char *p = fmt; *p; p++) {
        if (*p != '%') {
            out_char(*p);
            continue;
        }

        int width = 0;
        char pad = ' ';

        p++;
        if (!*p) {
            out_char('%');
            break;
        }

        if (*p == '0') {
            pad = '0';
            p++;
        }

        while (*p >= '0' && *p <= '9') {
            width = width * 10 + (*p - '0');
            p++;
        }

        switch (*p) {
        case 's': {
            const char *s = va_arg(args, const char *);
            out_str(s);
            break;
        }
        case 'x': {
            uint32_t v = va_arg(args, uint32_t);
            print_hex(v, width ? width : 1, pad);
            break;
        }
        case 'd': {
            int v = va_arg(args, int);
            print_int(v, width, pad);
            break;
        }
        case 'u': {
            uint32_t v = va_arg(args, uint32_t);
            print_uint(v, width, pad);
            break;
        }
        case 'c': {
            int c = va_arg(args, int);
            out_char((char)c);
            break;
        }
        case '%':
            out_char('%');
            break;
        default:
            out_char('%');
            out_char(*p);
            break;
        }
    }

    va_end(args);
    return 0;
}
