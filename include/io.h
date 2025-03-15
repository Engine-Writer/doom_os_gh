#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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

#define FLAG_SET(x, flag) x |= (flag)
#define FLAG_UNSET(x, flag) x &= ~(flag)

#define UNUSED_PORT 0x80

uint32_t inl(uint16_t port);
void outl(uint16_t port, uint32_t data);

uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t data);

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t value);

static inline void insw(uint16_t __port, void *__buf, unsigned long __n) {
	asm("cld; rep; insw"
			: "+D"(__buf), "+c"(__n)
			: "d"(__port));
}

static inline void outsw(uint16_t __port, const void *__buf, unsigned long __n) {
	asm("cld; rep; outsw"
			: "+S"(__buf), "+c"(__n)
			: "d"(__port));
}

void iowait();

void cpuSetMSR(uint32_t msr, uint32_t eax, uint32_t edx);
void cpuGetMSR(uint32_t msr, uint32_t *eax, uint32_t *edx);
uint8_t apic_enablable();

extern void HALT();
bool __attribute__((cdecl)) Disk_GetDriveParams(uint8_t drive,
        uint8_t* driveTypeOut,
        uint16_t* cylindersOut,
        uint16_t* sectorsOut,
        uint16_t* headsOut
);

bool __attribute__((cdecl)) Disk_Reset(uint8_t drive);

bool __attribute__((cdecl)) Disk_Read(uint8_t drive,
        uint16_t cylinder,
        uint16_t sector,
        uint16_t head,
        uint8_t count,
        void* lowerDataOut
);

extern char *read_buffer;

#endif // IO_H
