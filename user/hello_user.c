#include <stdint.h>

#include "osmosis/syscall_numbers.h"

static inline int32_t syscall1(uint32_t num, uint32_t arg1) {
    int32_t ret;
    __asm__ __volatile__("int $0x80"
                         : "=a"(ret)
                         : "a"(num), "b"(arg1)
                         : "memory");
    return ret;
}

static inline int32_t syscall3(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    int32_t ret;
    __asm__ __volatile__("int $0x80"
                         : "=a"(ret)
                         : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3)
                         : "memory");
    return ret;
}

void _start(void) {
    const char message[] = "hello from userland\n";
    (void)syscall3(OSMOSIS_SYS_WRITE, 1, (uint32_t)(uintptr_t)message, sizeof(message) - 1);
    syscall1(OSMOSIS_SYS_EXIT, 0);
    for (;;)
        ;
}
