#ifndef OSMOSIS_ARCH_I386_PIT_H
#define OSMOSIS_ARCH_I386_PIT_H

#include <stdint.h>

/*
 * Programmable Interval Timer (PIT) setup and tick accounting. The PIT is the
 * first timing primitive the kernel uses; higher-level schedulers can later
 * build on the tick counter it maintains.
 */
void pit_init(uint32_t frequency_hz);
uint32_t pit_ticks(void);
void pit_wait_ticks(uint32_t delta);

#endif
