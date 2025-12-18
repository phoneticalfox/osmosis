#include "osmosis/panic.h"
#include "osmosis/kprintf.h"

__attribute__((noreturn)) void panic(const char *message) {
    kprintf("PANIC: %s\n", message);
    for (;;) {
        __asm__ __volatile__("cli; hlt");
    }
}
