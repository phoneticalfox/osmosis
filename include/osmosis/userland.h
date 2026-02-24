#ifndef OSMOSIS_USERLAND_H
#define OSMOSIS_USERLAND_H

#include <stdint.h>

#include "osmosis/arch/i386/isr.h"

struct process_image;

int userland_run_demo(void);
int userland_bootstrap_demo(void);
void userland_finished(void);
int userland_load_elf_into(const uint8_t *image, uint32_t size, uint32_t *directory, struct process_image *out);
int userland_clone_region(uint32_t *src_directory, uint32_t *dst_directory, uintptr_t low, uintptr_t high);
int userland_user_range_ok(uintptr_t ptr, uint32_t len);
void userland_exit_from_syscall(struct isr_frame *frame, uint32_t code);
uint32_t userland_current_pid(void);

#endif
