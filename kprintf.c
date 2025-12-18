#include "kprintf.h"
#include "tty.h"

static void itoa_base(uint32_t value, char *buffer, int base) {
   const char digits[] = "0123456789ABCDEF";
   char temp[32];
   int i = 0;
   if (value == 0) { buffer[0] = '0'; buffer[1] = '\0'; return; }
   while (value > 0) {
       temp[i++] = digits[value % base];
       value /= base;
   }
   int j = 0;
   while (i > 0) buffer[j++] = temp[--i];
   buffer[j] = '\0';
}

void kvprintf(const char *fmt, va_list args) {
   char buffer[32];
   for (size_t i = 0; fmt[i] != '\0'; i++) {
       if (fmt[i] != '%') {
           tty_putc(fmt[i]);
           continue;
       }
       i++;
       switch (fmt[i]) {
           case 's': tty_write(va_arg(args, char*)); break;
           case 'c': tty_putc((char)va_arg(args, int)); break;
           case 'd': {
               int val = va_arg(args, int);
               if (val < 0) { tty_putc('-'); val = -val; }
               itoa_base((uint32_t)val, buffer, 10);
               tty_write(buffer);
               break;
           }
           case 'x':
               tty_write("0x");
               itoa_base(va_arg(args, uint32_t), buffer, 16);
               tty_write(buffer);
               break;
           default: tty_putc(fmt[i]); break;
       }
   }
}

void kprintf(const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);
   kvprintf(fmt, args);
   va_end(args);
}