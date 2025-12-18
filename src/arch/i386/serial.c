#include "osmosis/arch/i386/serial.h"
#include "osmosis/arch/i386/io.h"

#define COM1_PORT 0x3F8

static int serial_ready = 0;

void serial_init(void) {
    outb(COM1_PORT + 1, 0x00); // Disable interrupts
    outb(COM1_PORT + 3, 0x80); // Enable DLAB
    outb(COM1_PORT + 0, 0x03); // Divisor low byte (38400 baud)
    outb(COM1_PORT + 1, 0x00); // Divisor high byte
    outb(COM1_PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7); // Enable FIFO, clear, 14-byte threshold
    outb(COM1_PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
    serial_ready = 1;
}

static int serial_can_transmit(void) {
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_write_char(char c) {
    if (!serial_ready) {
        return;
    }

    while (!serial_can_transmit()) {
    }

    outb(COM1_PORT, (uint8_t)c);
}

void serial_write(const char *s) {
    if (!serial_ready) {
        return;
    }

    for (; *s; s++) {
        serial_write_char(*s);
    }
}

int serial_is_initialized(void) {
    return serial_ready;
}
