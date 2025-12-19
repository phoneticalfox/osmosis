#ifndef OSMOSIS_ARCH_I386_PIT_H
#define OSMOSIS_ARCH_I386_PIT_H

#include <stdint.h>

/*
 * Programmable Interval Timer (PIT) setup and tick accounting. The PIT is the
 * first timing primitive the kernel uses; higher-level schedulers can later
 * build on the tick counter it maintains.
 */
void pit_init(uint32_t frequency_hz);
uint32_t pit_frequency(void);
uint32_t pit_ticks(void);
void pit_wait_ticks(uint32_t delta);
uint64_t pit_uptime_ms(void);
void pit_sleep_ms(uint32_t ms);

struct pit_health {
    uint32_t last_snapshot;
    uint32_t last_delta;
    int stalled;
};

void pit_health_poll(void);
struct pit_health pit_health_latest(void);

#endif
