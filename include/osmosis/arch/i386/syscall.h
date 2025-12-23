#ifndef OSMOSIS_ARCH_I386_SYSCALL_H
#define OSMOSIS_ARCH_I386_SYSCALL_H

#include <stdint.h>

#include "osmosis/arch/i386/isr.h"
#include "osmosis/syscall_numbers.h"

enum syscall_number {
    SYSCALL_WRITE = OSMOSIS_SYS_WRITE,
    SYSCALL_EXIT  = OSMOSIS_SYS_EXIT,
    SYSCALL_GETPID = OSMOSIS_SYS_GETPID,
    SYSCALL_BRK = OSMOSIS_SYS_BRK,
};

void syscall_init(void);
void syscall_handler(struct isr_frame *frame);

#endif
