#include <stdint.h>

#include "osmosis/syscall_numbers.h"

static inline int32_t syscall0(uint32_t num) {
    int32_t ret;
    __asm__ __volatile__("int $0x80"
                         : "=a"(ret)
                         : "a"(num)
                         : "memory");
    return ret;
}

static inline int32_t syscall1(uint32_t num, uint32_t arg1) {
    int32_t ret;
    __asm__ __volatile__("int $0x80"
                         : "=a"(ret)
                         : "a"(num), "b"(arg1)
                         : "memory");
    return ret;
}

static inline int32_t syscall2(uint32_t num, uint32_t arg1, uint32_t arg2) {
    int32_t ret;
    __asm__ __volatile__("int $0x80"
                         : "=a"(ret)
                         : "a"(num), "b"(arg1), "c"(arg2)
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
    const char parent_msg[] = "hello from parent\n";
    const char child_msg[] = "hello from child\n";

    int32_t pid = syscall0(OSMOSIS_SYS_FORK);
    if (pid == 0) {
        syscall3(OSMOSIS_SYS_WRITE, 1, (uint32_t)(uintptr_t)child_msg, sizeof(child_msg) - 1);
        syscall1(OSMOSIS_SYS_EXIT, 42);
    } else {
        syscall3(OSMOSIS_SYS_WRITE, 1, (uint32_t)(uintptr_t)parent_msg, sizeof(parent_msg) - 1);
        syscall2(OSMOSIS_SYS_WAITPID, (uint32_t)pid, 0);
        syscall1(OSMOSIS_SYS_EXIT, 0);
    }
}
