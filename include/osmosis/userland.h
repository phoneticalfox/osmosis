#ifndef OSMOSIS_USERLAND_H
#define OSMOSIS_USERLAND_H

#include <stdint.h>

#include "osmosis/process.h"

int userland_load_elf_into(const uint8_t *image, uint32_t size, uint32_t *page_directory, struct process_image *out);
int userland_clone_region(uint32_t *src_dir, uint32_t *dst_dir, uintptr_t low, uintptr_t high);

int userland_bootstrap_demo(void);

#endif
