#ifndef OSMOSIS_ARCH_I386_KEYBOARD_H
#define OSMOSIS_ARCH_I386_KEYBOARD_H

#include <stdint.h>

/*
 * The keyboard driver listens to IRQ1 and reports set-1 scancodes.
 * "pressed" is non-zero on key press, zero on release.
 */
typedef void (*keyboard_handler_t)(uint8_t scancode, int pressed);

void keyboard_init(void);
void keyboard_set_handler(keyboard_handler_t handler);

#endif
