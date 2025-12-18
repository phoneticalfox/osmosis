#include "osmosis/arch/i386/keyboard.h"
#include "osmosis/arch/i386/io.h"
#include "osmosis/arch/i386/irq.h"

#define PS2_DATA    0x60
#define PS2_STATUS  0x64
#define KEYBOARD_BUFFER_CAPACITY 128

static keyboard_handler_t keyboard_handler = 0;
static char key_buffer[KEYBOARD_BUFFER_CAPACITY];
static volatile size_t buffer_head = 0;
static volatile size_t buffer_tail = 0;
static volatile size_t buffer_count = 0;

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
            if (buffer_count < KEYBOARD_BUFFER_CAPACITY) {
                key_buffer[buffer_tail] = c;
                buffer_tail = (buffer_tail + 1) % KEYBOARD_BUFFER_CAPACITY;
                buffer_count++;
            }
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
    buffer_head = buffer_tail = buffer_count = 0;
    irq_install_handler(1, keyboard_irq);
}

size_t keyboard_buffer_count(void) {
    uint32_t flags = irq_save();
    size_t count = buffer_count;
    irq_restore(flags);
    return count;
}

int keyboard_buffer_read(char *out_char) {
    if (!out_char) {
        return 0;
    }

    uint32_t flags = irq_save();
    if (buffer_count == 0) {
        irq_restore(flags);
        return 0;
    }

    *out_char = key_buffer[buffer_head];
    buffer_head = (buffer_head + 1) % KEYBOARD_BUFFER_CAPACITY;
    buffer_count--;
    irq_restore(flags);
    return 1;
}

void keyboard_buffer_clear(void) {
    uint32_t flags = irq_save();
    buffer_head = buffer_tail = buffer_count = 0;
    irq_restore(flags);
}
