#ifndef OSMOSIS_USERLAND_H
#define OSMOSIS_USERLAND_H

#include <stdint.h>

#include "osmosis/arch/i386/isr.h"

int userland_run_demo(void);
int userland_user_range_ok(uintptr_t ptr, uint32_t len);
void userland_exit_from_syscall(struct isr_frame *frame, uint32_t code);
uint32_t userland_current_pid(void);

#endif
