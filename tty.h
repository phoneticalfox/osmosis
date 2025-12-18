#ifndef _TTY_H_
#define _TTY_H_
#include <stddef.h>
#include <stdint.h>

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000

typedef enum {
   COLOR_BLACK = 0, COLOR_BLUE = 1, COLOR_GREEN = 2, COLOR_CYAN = 3,
   COLOR_RED = 4, COLOR_MAGENTA = 5, COLOR_BROWN = 6, COLOR_LIGHT_GREY = 7,
   COLOR_DARK_GREY = 8, COLOR_LIGHT_BLUE = 9, COLOR_LIGHT_GREEN = 10,
   COLOR_LIGHT_CYAN = 11, COLOR_LIGHT_RED = 12, COLOR_LIGHT_MAGENTA = 13,
   COLOR_LIGHT_BROWN = 14, COLOR_WHITE = 15
} vga_color_t;

typedef struct {
   uint16_t *buffer;
   uint8_t color;
   size_t x;
   size_t y;
} tty_t;

extern tty_t tty;

void tty_init(void);
void tty_set_color(uint8_t fg, uint8_t bg);
void tty_putc(char c);
void tty_write(const char *str);
void tty_clear(void);
#endif