#include "panic.h"
#include "kprintf.h"
#include "tty.h"
#include <stdarg.h>

void panic(const char *fmt, ...) {
   tty_set_color(COLOR_WHITE, COLOR_RED);
   tty_clear();
   kprintf("************************************************\n");
   kprintf("*** KERNEL PANIC                  ***\n");
   kprintf("************************************************\n\n");
   
   va_list args;
   va_start(args, fmt);
   kvprintf(fmt, args);
   va_end(args);
   
   kprintf("\n\nSystem Halted.");
   __asm__ __volatile__ ("cli; hlt");
   for(;;);
}