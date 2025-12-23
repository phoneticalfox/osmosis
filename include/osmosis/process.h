#ifndef OSMOSIS_PROCESS_H
#define OSMOSIS_PROCESS_H

#include <stdint.h>

#include "osmosis/arch/i386/isr.h"

enum process_state {
    PROCESS_UNUSED = 0,
    PROCESS_RUNNABLE,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_ZOMBIE
};

struct process_image {
    uintptr_t entry;
    uintptr_t lowest;
    uintptr_t highest;
    uintptr_t stack_base;
    uintptr_t stack_top;
};

struct process {
    uint32_t pid;
    uint32_t parent_pid;
    enum process_state state;
    struct isr_frame context;
    struct process_image image;
    int exit_status;
    int waiting_for;
    uint32_t *page_directory;
    char name[32];
};

void process_init(void);
int process_spawn_from_image(const uint8_t *image, uint32_t size, const char *name);
int process_schedule(struct isr_frame *frame);
void process_prepare_kernel_return(struct isr_frame *frame);
int process_enter_first(void);
void process_set_idle_callback(void (*cb)(void));
uint32_t *process_kernel_directory(void);
struct process *process_current(void);
uint32_t process_current_pid(void);

int process_sys_fork(struct isr_frame *frame);
int process_sys_execve(struct isr_frame *frame, const char *path, const char *const *argv);
int process_sys_waitpid(struct isr_frame *frame, int pid);
void process_sys_exit(struct isr_frame *frame, int code);

int process_user_pointer_ok(uintptr_t ptr, uint32_t len);

/* Debug helpers */
void process_list(void);

#endif
