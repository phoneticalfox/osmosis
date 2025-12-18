#include "osmosis/arch/i386/keyboard.h"
#include "osmosis/arch/i386/io.h"
#include "osmosis/arch/i386/irq.h"
#include "osmosis/tty.h"

#define PS2_DATA    0x60
#define PS2_STATUS  0x64

static keyboard_handler_t keyboard_handler = 0;

static const char scancode_map[128] = {
    0,   27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,   0,   0, ' ',  0,
};

static void default_handler(uint8_t scancode, int pressed) {
    if (!pressed) {
        return;
    }

    if (scancode < sizeof(scancode_map)) {
        char c = scancode_map[scancode];
        if (c) {
            tty_putc(c);
        }
    }
}

static void keyboard_irq(struct isr_frame *frame) {
    (void)frame;

    /* Only handle if the controller reports data available. */
    uint8_t status = inb(PS2_STATUS);
    if (!(status & 0x01)) {
        return;
    }

    uint8_t scancode = inb(PS2_DATA);
    int pressed = !(scancode & 0x80);
    scancode &= 0x7F;

    keyboard_handler_t handler = keyboard_handler;
    if (handler) {
        handler(scancode, pressed);
    } else {
        default_handler(scancode, pressed);
    }
}

void keyboard_set_handler(keyboard_handler_t handler) {
    keyboard_handler = handler;
}

void keyboard_init(void) {
    keyboard_handler = default_handler;
    irq_install_handler(1, keyboard_irq);
}
