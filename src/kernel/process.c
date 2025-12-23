#include "osmosis/process.h"

#include <stddef.h>
#include <stdint.h>

#include "osmosis/arch/i386/paging.h"
#include "osmosis/arch/i386/segments.h"
#include "osmosis/kprintf.h"
#include "osmosis/pmm.h"
#include "osmosis/userland.h"
#include "osmosis/vfs.h"

#define MAX_PROCESSES 8
#define USER_CODE (USER_CODE_SELECTOR | 0x03)
#define USER_DATA (USER_DATA_SELECTOR | 0x03)

static struct process processes[MAX_PROCESSES];
static uint32_t next_pid = 1;
static struct process *current = NULL;
static uint32_t *kernel_directory = NULL;
static void (*idle_callback)(void) = NULL;

static struct process *alloc_process(const char *name) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROCESS_UNUSED) {
            struct process *p = &processes[i];
            p->pid = next_pid++;
            p->parent_pid = 0;
            p->state = PROCESS_RUNNABLE;
            p->exit_status = 0;
            p->waiting_for = -1;
            p->page_directory = NULL;
            for (int c = 0; c < 32; c++) {
                p->name[c] = 0;
            }
            if (name) {
                int c = 0;
                for (; c < 31 && name[c]; c++) {
                    p->name[c] = name[c];
                }
                p->name[c] = 0;
            }
            return p;
        }
    }
    return NULL;
}

static void setup_initial_context(struct process *p) {
    struct isr_frame *ctx = &p->context;
    for (size_t i = 0; i < sizeof(*ctx) / sizeof(uint32_t); i++) {
        ((uint32_t *)ctx)[i] = 0;
    }
    ctx->ds = USER_DATA;
    ctx->es = USER_DATA;
    ctx->fs = USER_DATA;
    ctx->gs = USER_DATA;
    ctx->ss = USER_DATA;
    ctx->cs = USER_CODE;
    ctx->eip = (uint32_t)p->image.entry;
    ctx->useresp = (uint32_t)p->image.stack_top;
    ctx->eflags = 0x202;
}

static struct process *find_runnable(void) {
    int start = 0;
    if (current) {
        start = (int)(current - processes) + 1;
    }
    for (int i = 0; i < MAX_PROCESSES; i++) {
        int idx = (start + i) % MAX_PROCESSES;
        if (processes[idx].state == PROCESS_RUNNABLE) {
            return &processes[idx];
        }
    }
    return NULL;
}

void process_init(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].state = PROCESS_UNUSED;
    }
    kernel_directory = paging_current_directory();
}

void process_set_idle_callback(void (*cb)(void)) {
    idle_callback = cb;
}

int process_spawn_from_image(const uint8_t *image, uint32_t size, const char *name) {
    struct process *p = alloc_process(name);
    if (!p) {
        kprintf("process: no free slots for %s\n", name ? name : "(anon)");
        return -1;
    }

    p->page_directory = paging_create_address_space();
    if (!p->page_directory) {
        kprintf("process: failed to allocate page directory\n");
        p->state = PROCESS_UNUSED;
        return -1;
    }

    struct process_image img;
    if (!userland_load_elf_into(image, size, p->page_directory, &img)) {
        kprintf("process: ELF load failed for %s\n", name ? name : "(anon)");
        p->state = PROCESS_UNUSED;
        return -1;
    }

    p->image = img;
    setup_initial_context(p);
    return (int)p->pid;
}

struct process *process_current(void) {
    return current;
}

uint32_t process_current_pid(void) {
    if (!current) {
        return 0;
    }
    return current->pid;
}

uint32_t *process_kernel_directory(void) {
    return kernel_directory;
}

static void save_current(struct isr_frame *frame) {
    if (!current || !frame) {
        return;
    }
    current->context = *frame;
    if (current->state == PROCESS_RUNNING) {
        current->state = PROCESS_RUNNABLE;
    }
}

static void enter_process(struct process *p, struct isr_frame *frame) {
    current = p;
    p->state = PROCESS_RUNNING;
    paging_switch_directory(p->page_directory);
    *frame = p->context;
}

int process_schedule(struct isr_frame *frame) {
    save_current(frame);
    struct process *next = find_runnable();
    if (!next) {
        return -1;
    }
    enter_process(next, frame);
    return 0;
}

static __attribute__((noreturn)) void process_halt_forever(void) {
    kprintf("process: all user tasks exited\n");
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}

void process_prepare_kernel_return(struct isr_frame *frame) {
    paging_switch_directory(kernel_directory);
    frame->ds = KERNEL_DATA_SELECTOR;
    frame->es = KERNEL_DATA_SELECTOR;
    frame->fs = KERNEL_DATA_SELECTOR;
    frame->gs = KERNEL_DATA_SELECTOR;
    frame->cs = KERNEL_CODE_SELECTOR;
    frame->ss = KERNEL_DATA_SELECTOR;
    frame->eflags |= 0x200;
    frame->useresp = (uint32_t)(uintptr_t)frame;
    if (idle_callback) {
        frame->eip = (uint32_t)(uintptr_t)idle_callback;
    } else {
        frame->eip = (uint32_t)(uintptr_t)process_halt_forever;
    }
}

int process_enter_first(void) {
    struct isr_frame frame;
    if (process_schedule(&frame) < 0) {
        return -1;
    }
    uint32_t data_sel = frame.ds;
    uint32_t code_sel = frame.cs;
    uint32_t eip = frame.eip;
    uint32_t user_stack = frame.useresp;
    __asm__ __volatile__(
        "cli\n"
        "mov %0, %%eax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "pushl %0\n"
        "pushl %1\n"
        "pushfl\n"
        "popl %%eax\n"
        "orl $0x200, %%eax\n"
        "pushl %%eax\n"
        "pushl %2\n"
        "pushl %3\n"
        "iret\n"
        :
        : "r"(data_sel), "r"(user_stack), "r"(code_sel), "r"(eip)
        : "eax");
    __builtin_unreachable();
}

int process_sys_fork(struct isr_frame *frame) {
    if (!current) {
        return -1;
    }

    struct process *child = alloc_process(current->name);
    if (!child) {
        kprintf("fork: no process slot\n");
        return -12;
    }
    child->parent_pid = current->pid;
    child->page_directory = paging_create_address_space();
    if (!child->page_directory) {
        child->state = PROCESS_UNUSED;
        return -12;
    }

    if (!userland_clone_region(current->page_directory, child->page_directory,
                               current->image.lowest, current->image.highest)) {
        kprintf("fork: failed to clone region\n");
        child->state = PROCESS_UNUSED;
        return -12;
    }
    child->image = current->image;

    child->context = *frame;
    child->context.eax = 0; /* child returns 0 */
    child->state = PROCESS_RUNNABLE;

    return (int)child->pid;
}

void process_sys_exit(struct isr_frame *frame, int code) {
    if (!current) {
        return;
    }
    current->exit_status = code;
    current->state = PROCESS_ZOMBIE;
    (void)frame;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        struct process *p = &processes[i];
        if (p->state == PROCESS_WAITING && p->waiting_for != 0 &&
            (p->waiting_for == -1 || p->waiting_for == (int)current->pid) &&
            p->pid == current->parent_pid) {
            p->context.eax = current->pid;
            p->state = PROCESS_RUNNABLE;
            p->waiting_for = -1;
        }
    }
}

int process_sys_waitpid(struct isr_frame *frame, int pid) {
    (void)frame;
    if (!current) {
        return -1;
    }
    int found = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        struct process *p = &processes[i];
        if (p->state == PROCESS_ZOMBIE && (pid == -1 || (int)p->pid == pid) && p->parent_pid == current->pid) {
            int ret = (int)p->pid;
            p->state = PROCESS_UNUSED;
            return ret;
        }
        if (p->state != PROCESS_UNUSED && p->parent_pid == current->pid) {
            found = 1;
        }
    }
    if (!found) {
        return -10; /* ECHILD */
    }
    current->state = PROCESS_WAITING;
    current->waiting_for = pid;
    return -16; /* EBUSY (retry later) */
}

int process_sys_execve(struct isr_frame *frame, const char *path, const char *const *argv) {
    (void)argv;
    if (!current || !path) {
        return -22;
    }
    const struct vfs_node *node = vfs_lookup(path);
    if (!node) {
        return -2; /* ENOENT */
    }
    struct process_image img;
    if (!userland_load_elf_into(node->data, node->size, current->page_directory, &img)) {
        return -8; /* ENOEXEC */
    }
    current->image = img;
    setup_initial_context(current);
    frame->eax = 0;
    enter_process(current, frame);
    return 0;
}

int process_user_pointer_ok(uintptr_t ptr, uint32_t len) {
    if (!current) {
        return 0;
    }
    if (len == 0) {
        return 0;
    }
    uintptr_t end = ptr + len - 1;
    if (end < ptr) {
        return 0;
    }
    if (ptr < current->image.lowest || end > current->image.highest) {
        return 0;
    }
    return paging_range_has_flags(ptr, len, PAGE_USER);
}

void process_list(void) {
    kprintf("PID  PPID  STATE     NAME\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        const struct process *p = &processes[i];
        if (p->state == PROCESS_UNUSED) {
            continue;
        }
        const char *state = "unk";
        switch (p->state) {
            case PROCESS_RUNNABLE:
                state = "runnable";
                break;
            case PROCESS_RUNNING:
                state = "running";
                break;
            case PROCESS_WAITING:
                state = "waiting";
                break;
            case PROCESS_ZOMBIE:
                state = "zombie";
                break;
            default:
                state = "unk";
                break;
        }
        kprintf("%-4u %-5u %-8s %s\n", p->pid, p->parent_pid, state, p->name);
    }
}
