#include "osmosis/arch/i386/isr.h"
#include "osmosis/kprintf.h"
#include "osmosis/panic.h"

static const char *exception_names[32] = {
    "Divide-by-zero", "Debug", "Non-maskable interrupt", "Breakpoint", "Overflow",
    "BOUND range exceeded", "Invalid opcode", "Device not available", "Double fault",
    "Coprocessor segment overrun", "Invalid TSS", "Segment not present", "Stack-segment fault",
    "General protection fault", "Page fault", "Reserved", "x87 floating-point exception",
    "Alignment check", "Machine check", "SIMD floating-point exception", "Virtualization exception",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Security exception", "Reserved"
};

void isr_handler(struct isr_frame *frame) {
    if (frame->int_no < 32) {
        const char *name = exception_names[frame->int_no];
        kprintf("\n*** CPU EXCEPTION ***\n");
        kprintf("Vector : %d (%s)\n", frame->int_no, name);
        kprintf("Error  : 0x%x\n", frame->err_code);
        kprintf("EIP    : 0x%x\n", frame->eip);
        kprintf("CS:EFLAGS: 0x%x:0x%x\n", frame->cs, frame->eflags);
        panic("Unhandled CPU exception");
    }
}
