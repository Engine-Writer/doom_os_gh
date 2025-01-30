#ifndef APIC_H
#define APIC_H

#include <stdint.h>
#include "isr.h"
#include "io.h"

// APIC Registers
#define APIC_EOI              0xB0  // End Of Interrupt Register
#define APIC_LID              0xD0
#define APIC_ISR_BASE         0x100 // In-Service Register Base
#define APIC_IRR_BASE         0x200 // Interrupt Request Register Base
#define APIC_TPR              0x080 // Task Priority Register
#define APIC_LVT_TIMER        0x320 // Local Vector Table Timer Register
#define APIC_LVT_LINT0        0x350 // Local Vector Table LINT0 Register
#define APIC_LVT_LINT1        0x360 // Local Vector Table LINT1 Register


#define APIC_BASE             0xFEE00000 // APIC Base Address (commonly defined, can vary)
#define APIC_REGISTER_OFFSET  0x10  // Offset to the registers in APIC
#define APIC_ID               0x20
#define CR4_APIC_BIT          0x200    // The bit in CR4 register for enabling APIC (bit 9)
#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

#define APIC_DELIVERY_MODE_FIXED     0x00000000  // Normal fixed priority delivery
#define APIC_DELIVERY_MODE_LOWEST    0x00000100  // Lowest priority delivery
#define APIC_DELIVERY_MODE_SMI       0x00000200  // System Management Interrupt
#define APIC_DELIVERY_MODE_NMI       0x00000400  // Non-Maskable Interrupt
#define APIC_DELIVERY_MODE_INIT      0x00000500  // INIT signal
#define APIC_DELIVERY_MODE_STARTUP   0x00000600  // Startup IPI (SIPI)
#define APIC_DELIVERY_MODE_EXTINT    0x00000700  // External interrupt

#define APIC_DELIVERY_MODE_MASK      0x00000700  // Mask for extracting delivery mode bits


#define APIC_SVR            0xF0  // Spurious Interrupt Vector Register
#define APIC_DEST_FORMAT    0xE0  // Destination Format Register


extern uint32_t apic_base;

// Function to read CR4 register
static inline uint32_t ReadCR4() {
    uint32_t cr4;
    asm("mov %%cr4, %0" : "=r"(cr4));
    return cr4;
}

// Function to write CR4 register
static inline void WriteCR4(uint32_t cr4) {
    asm("mov %0, %%cr4" :: "r"(cr4));
}

static inline void APIC_Write(uint32_t reg, uint32_t value) {
    *((volatile uint32_t *)(apic_base + reg)) = value;
}

static inline uint32_t APIC_Read(uint32_t reg) {
    return *((volatile uint32_t *)(apic_base + reg));
}

// APIC Interrupt Control

// Functions for setting and getting the APIC base address
void cpu_set_apic_base(uintptr_t apic);
uintptr_t cpu_get_apic_base();

// Functions to read and write to the IOAPIC registers
uint32_t cpuReadIoApic(void *ioapicaddr, uint32_t reg);
void cpuWriteIoApic(void *ioapicaddr, uint32_t reg, uint32_t value);

// Function to configure the IMCR to switch from PIC to APIC mode
void ConfigureIMCR();
void APIC_Initialize();
void APIC_SendEOI();


#endif // APIC_H
