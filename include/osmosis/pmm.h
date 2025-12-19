#ifndef OSMOSIS_PMM_H
#define OSMOSIS_PMM_H

#include <stdint.h>

#include "osmosis/boot.h"

void pmm_init(const struct boot_info *boot);
uintptr_t pmm_alloc_frame(void);
void pmm_free_frame(uintptr_t addr);
uint32_t pmm_total_frames(void);
uint32_t pmm_free_frames(void);

#endif
