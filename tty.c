#include "tty.h"

tty_t tty;

static uint8_t vga_entry_color(vga_color_t fg, vga_color_t bg) {
   return fg | (bg << 4);
}

static uint16_t vga_entry(char c, uint8_t color) {
   return (uint16_t)c | (uint16_t)color << 8;
}

static void tty_scroll(void) {
   for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
       for (size_t x = 0; x < VGA_WIDTH; x++) {
           tty.buffer[y * VGA_WIDTH + x] = tty.buffer[(y + 1) * VGA_WIDTH + x];
       }
   }
   for (size_t x = 0; x < VGA_WIDTH; x++) {
       tty.buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', tty.color);
   }
   tty.y = VGA_HEIGHT - 1;
}

void tty_init(void) {
   tty.buffer = (uint16_t *)VGA_MEMORY;
   tty.x = 0;
   tty.y = 0;
   tty.color = vga_entry_color(COLOR_LIGHT_GREY, COLOR_BLACK);
   tty_clear();
}

void tty_clear(void) {
   for (size_t y = 0; y < VGA_HEIGHT; y++) {
       for (size_t x = 0; x < VGA_WIDTH; x++) {
           tty.buffer[y * VGA_WIDTH + x] = vga_entry(' ', tty.color);
       }
   }
   tty.x = 0; tty.y = 0;
}

void tty_putc(char c) {
   if (c == '\n') {
       tty.x = 0;
       tty.y++;
   } else if (c == '\r') {
       tty.x = 0;
   } else {
       tty.buffer[tty.y * VGA_WIDTH + tty.x] = vga_entry(c, tty.color);
       tty.x++;
       if (tty.x >= VGA_WIDTH) {
           tty.x = 0;
           tty.y++;
       }
   }
   if (tty.y >= VGA_HEIGHT) tty_scroll();
}

void tty_write(const char *str) {
   for (size_t i = 0; str[i] != '\0'; i++) tty_putc(str[i]);
}

void tty_set_color(uint8_t fg, uint8_t bg) {
   tty.color = vga_entry_color(fg, bg);
}