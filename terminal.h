#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>
#include <stdint.h>
#include "util.h"

// VGA Text Mode dimensions
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY (uint16_t*)0xB8000

#define VGA_PORT_INDEX 0x3D4
#define VGA_PORT_DATA  0x3D5

// VGA color constants (Foreground/Background)
typedef enum {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
} VGA_Color;

typedef uint8_t VGA_Color2;
typedef uint16_t VGA_Entry;

// Function prototypes for terminal handling
// Inlines
static inline VGA_Color2 make_color(VGA_Color fg, VGA_Color bg) {
    return (uint8_t)fg | (uint8_t)bg << 4;
}

static inline VGA_Entry make_vga_entry(char c, VGA_Color2 color) {
    return (uint8_t)c | (uint8_t)color << 8;
}

// C stuff
void terminal_initialize();
void terminal_setcolor(VGA_Color2 color);
void terminal_putentryat(char c, VGA_Color2 color, size_t x, size_t y);
void terminal_scroll();
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_clear(VGA_Color2 color);
void terminal_printf(const char *format, ...);

void terminal_set_cursor_position(uint8_Vector2 position);
uint8_Vector2 terminal_get_cursor_position();

//extern void (*cls)(VGA_Color2 color);
//extern void (*printf)(const char *format, ...);


#endif // TERMINAL_H