#include "osmosis/kprintf.h"
#include "osmosis/tty.h"
#include "osmosis/arch/i386/serial.h"

#include <stdarg.h>
#include <stdint.h>

static void print_hex(uint32_t value) {
    const char *hex_digits = "0123456789ABCDEF";
    char buffer[9];

    for (int i = 7; i >= 0; i--) {
        buffer[i] = hex_digits[value & 0xF];
        value >>= 4;
    }

    buffer[8] = '\0';
    tty_write(buffer);
    serial_write(buffer);
}

static void print_dec(int value) {
    char buffer[12];
    int idx = 0;
    int is_negative = value < 0;

    if (value == 0) {
        tty_putc('0');
        return;
    }

    if (is_negative) {
        value = -value;
    }

    while (value > 0 && idx < 11) {
        buffer[idx++] = '0' + (value % 10);
        value /= 10;
    }

    if (is_negative) {
        buffer[idx++] = '-';
    }

    for (int i = idx - 1; i >= 0; i--) {
        tty_putc(buffer[i]);
    }
}

int kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (const char *p = fmt; *p; p++) {
        if (*p != '%') {
            tty_putc(*p);
            continue;
        }

        p++;
        switch (*p) {
            case 's': {
                const char *s = va_arg(args, const char *);
                tty_write(s);
                break;
            }
            case 'x': {
                uint32_t v = va_arg(args, uint32_t);
                print_hex(v);
                break;
            }
            case 'd': {
                int v = va_arg(args, int);
                print_dec(v);
                break;
            }
            case 'c': {
                int c = va_arg(args, int);
                tty_putc((char)c);
                break;
            }
            case '%':
                tty_putc('%');
                break;
            default:
                tty_putc('%');
                tty_putc(*p);
                break;
        }
    }

    va_end(args);
    return 0;
}
