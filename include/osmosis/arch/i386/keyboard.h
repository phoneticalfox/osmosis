#ifndef OSMOSIS_ARCH_I386_KEYBOARD_H
#define OSMOSIS_ARCH_I386_KEYBOARD_H

#include <stddef.h>
#include <stdint.h>

/*
 * The keyboard driver listens to IRQ1 and reports set-1 scancodes.
 * "pressed" is non-zero on key press, zero on release.
 */
typedef void (*keyboard_handler_t)(uint8_t scancode, int pressed);

void keyboard_init(void);
void keyboard_set_handler(keyboard_handler_t handler);

/* Buffered character path for shell-style input. */
size_t keyboard_buffer_count(void);
int keyboard_buffer_read(char *out_char);
void keyboard_buffer_clear(void);

#endif
