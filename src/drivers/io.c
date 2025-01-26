#include "io.h"
#include <stdint.h>

uint16_t inw(uint16_t port) {
    uint16_t r;
    asm("inw %1, %0" : "=a" (r) : "dN" (port));
    return r;
}

void outw(uint16_t port, uint16_t data) {
    asm("outw %1, %0" : : "dN" (port), "a" (data));
}

// Inline static function to read a byte from a port
uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ __volatile__ (
        "inb %1, %0"
        : "=a"(result)
        : "d"(port)
    );
    return result;
}

// Inline static function to write a byte to a port
void outb(uint16_t port, uint8_t value) {
    __asm__ __volatile__ (
        "outb %0, %1"
        : 
        : "a"(value), "d"(port)
    );
}

void iowait() {
    outb(UNUSED_PORT, 0);
}
