#ifndef IO_H
#define IO_H

#include <stdint.h>

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define __MIN_IMPL(_x, _y, _xn, _yn) __extension__({\
        __typeof__(_x) _xn = (_x);\
        __typeof__(_y) _yn = (_y);\
        (_xn < _yn ? _xn : _yn);\
        })
#define MIN(_x, _y) __MIN_IMPL(_x, _y, CONCAT(__x, __COUNTER__), CONCAT(__y, __COUNTER__))

#define __MAX_IMPL(_x, _y, _xn, _yn) __extension__({\
        __typeof__(_x) _xn = (_x);\
        __typeof__(_y) _yn = (_y);\
        (_xn > _yn ? _xn : _yn);\
        })
#define MAX(_x, _y) __MAX_IMPL(_x, _y, CONCAT(__x, __COUNTER__), CONCAT(__y, __COUNTER__))

#define CLAMP(_x, _mi, _ma) (MAX(_mi, MIN(_x, _ma)))

// returns the highest set bit of x
// i.e. if x == 0xF, HIBIT(x) == 3 (4th index)
// WARNING: currently only works for up to 32-bit types
#define HIBIT(_x) (31 - __builtin_clz((_x)))

// returns the lowest set bit of x
#define LOBIT(_x)\
    __extension__({ __typeof__(_x) __x = (_x); HIBIT(__x & -__x); })

// returns _v with _n-th bit = _x
#define BIT_SET(_v, _n, _x) __extension__({\
        __typeof__(_v) __v = (_v);\
        (__v ^ ((-(_x) ^ __v) & (1 << (_n))));\
        })

#define PACKED __attribute__((packed))

#ifndef asm
#define asm __asm__ volatile
#endif

#define CLI() asm ("cli")
#define STI() asm ("sti")

static inline uint16_t inw(uint16_t port) {
    uint16_t r;
    asm("inw %1, %0" : "=a" (r) : "dN" (port));
    return r;
}

static inline void outw(uint16_t port, uint16_t data) {
    asm("outw %1, %0" : : "dN" (port), "a" (data));
}

// Inline static function to read a byte from a port
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ __volatile__ (
        "inb %1, %0"
        : "=a"(result)
        : "d"(port)
    );
    return result;
}

// Inline static function to write a byte to a port
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ __volatile__ (
        "outb %0, %1"
        : 
        : "a"(value), "d"(port)
    );
}

#endif // IO_H
