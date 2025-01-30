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
    asm (
        "inb %1, %0"
        : "=a"(result)
        : "d"(port)
    );
    return result;
}

// Inline static function to write a byte to a port
void outb(uint16_t port, uint8_t value) {
    asm (
        "outb %0, %1"
        : 
        : "a"(value), "d"(port)
    );
}

void iowait() {
    outb(UNUSED_PORT, 0);
}

void cpuGetMSR(uint32_t msr, uint32_t *eax, uint32_t *edx) {
    // Inline assembly to read from the MSR
    asm (
        "rdmsr" // MSR read instruction
        : "=a"(*eax), "=d"(*edx) // Output: EAX and EDX will store the 64-bit MSR value
        : "c"(msr) // Input: MSR to read from
    );
}

void cpuSetMSR(uint32_t msr, uint32_t eax, uint32_t edx) {
    // Inline assembly to write to the MSR
    asm (
        "wrmsr" // MSR write instruction
        : // No outputs
        : "c"(msr), "a"(eax), "d"(edx) // MSR, EAX (lower 32 bits), EDX (upper 32 bits)
    );
}

uint8_t apic_enablable() {
    uint32_t edx;
    
    // CPUID with EAX = 1 to get feature information
    __asm__ volatile (
        "cpuid"
        : "=d"(edx)      // Store EDX result (APIC bit is bit 9)
        : "a"(1)        // Set EAX = 1 (Feature Information)
        : "ecx", "ebx"  // Clobbered registers
    );
    
    // Check if APIC is supported by the CPU (bit 9)
    if ((edx & (1 << 9)) == 0) {
        return 0;  // APIC is not supported by the CPU
    }

    // Get the APIC base address from MSR 0x1B
    uint32_t eax, edx_base;
    cpuGetMSR(0x1B, &eax, &edx_base);  // APIC Base MSR

    // Check if the APIC is currently enabled (bit 11)
    if (edx_base & (1 << 11)) {
        return 1;  // APIC is enabled
    }

    // Check if APIC can be enabled by writing to the MSR (if bit 11 is not set)
    if ((edx_base & (1 << 10)) != 0) {
        // If bit 10 is set, it means the system can enable the APIC by writing to MSR
        return 2;  // APIC is not enabled, but it can be enabled
    }

    return 3;  // APIC is not enabled, and cannot be enabled
}
