// hpet.h

#ifndef HPET_H
#define HPET_H

#include "io.h"
#include "acpi.h"
#include <stdint.h>

// HPET Base Address
#define HPET_BASE_ADDRESS 0xFED00000

// HPET Register Offsets
#define HPET_GENERAL_CONFIGURATION   0x10
#define HPET_MAIN_COUNTER            0xF0
#define HPET_MMAP_SIZE               1024
#define HPET_GENERAL_CAPABILITIES_ID 0x0
#define HPET_MAIN_COUNTER            0xF0
#define HPET_TIMER_CONFIG            0x100
#define HPET_TIMER_COMPARATOR        0x108
#define HPET_ENABLE_BIT              0x1
#define HPET_FREQ                    14318180
#define HPET_TPS                     2500000

#define FIXED_POINT_SCALE (1ULL << 20)


extern uint32_t hpet_base_address;
extern volatile void *hpet_virt_addr;
extern uint64_t hpet_io_port;


// Function Prototypes
void HPET_Initialize();
void HPET_ConfigureTimer(uint32_t timer_id, uint64_t period_ns);
void HPET_IncrementValueAtInterval(uint32_t interval_ticks);
void HPET_Sleep(float seconds);
void HPET_SleepNS(uint32_t ns);


// Read a 32-bit value from an HPET register (works for both memory-mapped and I/O mode)
static inline uint32_t HPET_ReadIO(uint32_t offset) {
    if (hpet_data->base_address.AddressSpace == 0) {
        return *((volatile uint32_t *)((uint8_t *)hpet_virt_addr + offset));
    } else if (hpet_data->base_address.AddressSpace == 1) {
        return inl((uint16_t)(hpet_io_port + offset));
    }
    return 0;
}

// Write a 32-bit value to an HPET register (works for both memory-mapped and I/O mode)
static inline void HPET_WriteIO(uint32_t offset, uint32_t value) {
    if (hpet_data->base_address.AddressSpace == 0) {
        // Memory-mapped access
        *((volatile uint32_t *)((uint8_t *)hpet_virt_addr + offset)) = value;
    } else if (hpet_data->base_address.AddressSpace == 1) {
        // I/O-mapped access; assume outl() exists.
        outl((uint16_t)(hpet_io_port + offset), value);
    }
}



// Helper: Read full 64-bit HPET capabilities register.
static inline uint64_t HPET_ReadCapabilities() {
    uint32_t lower = HPET_ReadIO(HPET_GENERAL_CAPABILITIES_ID);
    uint32_t upper = HPET_ReadIO(HPET_GENERAL_CAPABILITIES_ID + 4);
    return (((uint64_t)upper) << 32) | lower;
}

// Read the HPET main counter (64-bit value)
static inline uint64_t HPET_ReadCounter() {
    uint32_t lower = HPET_ReadIO(HPET_MAIN_COUNTER);
    uint32_t upper = HPET_ReadIO(HPET_MAIN_COUNTER + 4);
    return ((uint64_t)upper << 32) | lower;
}

#endif // HPET_H
