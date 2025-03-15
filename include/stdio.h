#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include "terminal.h"
#include "memory.h"
#include "util.h"
#include "io.h"

typedef struct {
    uint8_t id;
    uint16_t cylinders;
    uint16_t sectors;
    uint16_t heads;
} disk_t;


void clrscr();
void scrollback(uint8_t lines);
void printf_unsigned(uint32_t number, int32_t radix);
void printf_signed(int32_t number, int32_t radix);
void printf(const char* fmt, ...);
void print_buffer(const char* msg, const void* buffer, uint32_t count);
void putc(char c);
void puts(const char *s);

#endif // STDIO_H;