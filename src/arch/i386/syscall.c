#include "osmosis/arch/i386/syscall.h"

#include "osmosis/arch/i386/idt.h"
#include "osmosis/arch/i386/segments.h"
#include "osmosis/kprintf.h"
#include "osmosis/arch/i386/serial.h"
#include "osmosis/tty.h"
#include "osmosis/userland.h"

#include <stddef.h>
#include <stdint.h>

extern void syscall_stub(void);

#define SYSCALL_EBADF 9
#define SYSCALL_EFAULT 14
#define SYSCALL_EINVAL 22
#define SYSCALL_ENOSYS 38

typedef uint32_t (*syscall_fn_t)(struct isr_frame *frame);

static uint32_t syscall_write(struct isr_frame *frame);
static uint32_t syscall_exit(struct isr_frame *frame);
static uint32_t syscall_getpid(struct isr_frame *frame);
static uint32_t syscall_brk(struct isr_frame *frame);

static const syscall_fn_t syscall_table[] = {
    [SYSCALL_WRITE] = syscall_write,
    [SYSCALL_EXIT] = syscall_exit,
    [SYSCALL_GETPID] = syscall_getpid,
    [SYSCALL_BRK] = syscall_brk,
};

static int32_t syscall_error(int code, const char *context, uint32_t eax, uint32_t eip) {
    kprintf("syscall error: %s (code=%d, eax=%u, eip=0x%x)\n", context, code, eax, eip);
    return -code;
}

void syscall_init(void) {
    uint8_t flags = 0x80 | 0x0E | 0x60; /* present | 32-bit gate | DPL=3 */
    idt_set_gate(0x80, (uint32_t)(uintptr_t)syscall_stub, KERNEL_CODE_SELECTOR, flags);
    kprintf("Syscall: int 0x80 registered (DPL=3).\n");
}

void syscall_handler(struct isr_frame *frame) {
    uint32_t num = frame->eax;
    if (num >= (sizeof(syscall_table) / sizeof(syscall_table[0])) || !syscall_table[num]) {
        frame->eax = (uint32_t)syscall_error(SYSCALL_ENOSYS, "unknown syscall", num, frame->eip);
        return;
    }

    uint32_t result = syscall_table[num](frame);
    frame->eax = result;
}

static uint32_t syscall_write(struct isr_frame *frame) {
    uint32_t fd = frame->ebx;
    const char *buf = (const char *)(uintptr_t)frame->ecx;
    uint32_t len = frame->edx;

    if (fd != 1) {
        return (uint32_t)syscall_error(SYSCALL_EBADF, "write: unsupported fd", frame->eax, frame->eip);
    }
    if (!buf || len == 0) {
        return (uint32_t)syscall_error(SYSCALL_EINVAL, "write: empty buffer", frame->eax, frame->eip);
    }
    if (!userland_user_range_ok((uintptr_t)buf, len)) {
        return (uint32_t)syscall_error(SYSCALL_EFAULT, "write: invalid user range", frame->eax, frame->eip);
    }

    for (uint32_t i = 0; i < len; i++) {
        char c = buf[i];
        tty_putc(c);
        serial_write_char(c);
    }
    return len;
}

static uint32_t syscall_exit(struct isr_frame *frame) {
    uint32_t code = frame->ebx;
    userland_exit_from_syscall(frame, code);
    return code;
}

static uint32_t syscall_getpid(struct isr_frame *frame) {
    (void)frame;
    return userland_current_pid();
}

static uint32_t syscall_brk(struct isr_frame *frame) {
    (void)frame;
    return (uint32_t)syscall_error(SYSCALL_ENOSYS, "brk/sbrk placeholder", frame->eax, frame->eip);
}
