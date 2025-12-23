#ifndef OSMOSIS_SHELL_H
#define OSMOSIS_SHELL_H

/* Minimal kernel-side shell to exercise OS-facing interfaces. */
struct boot_info;

void shell_init(const struct boot_info *boot);
void shell_run(void);

#endif
