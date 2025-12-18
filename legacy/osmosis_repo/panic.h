#ifndef _PANIC_H_
#define _PANIC_H_
__attribute__((noreturn))
void panic(const char *fmt, ...);
#endif