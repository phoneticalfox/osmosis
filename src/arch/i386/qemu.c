#include "osmosis/arch/i386/qemu.h"
#include "osmosis/arch/i386/io.h"

#define QEMU_EXIT_PORT 0xF4

void qemu_exit(uint32_t code) {
    outb(QEMU_EXIT_PORT, (uint8_t)code);
}
