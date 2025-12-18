#ifndef _KPRINTF_H_
#define _KPRINTF_H_
#include <stdarg.h>
#include <stdint.h>
void kprintf(const char *fmt, ...);
void kvprintf(const char *fmt, va_list args);
#endif