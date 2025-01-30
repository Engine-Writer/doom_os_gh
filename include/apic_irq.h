#ifndef APIC_IRQ_H
#define APIC_IRQ_H

#include "pic_irq.h"
#include "io.h"
#include "terminal.h"

// Global IRQ Handlers array
extern IRQHandler g_APICIRQHandlers[64];  // Max of 64 IRQs

// APIC IRQ Functions
void APIC_IRQ_Handler(Registers* regs);       // Interrupt handler for APIC IRQs
void APIC_IRQ_Initialize();                   // Initialize APIC IRQ handling
void APIC_IRQ_RegisterHandler(uint32_t irq, IRQHandler handler);  // Register an IRQ handler for APIC IRQs
void APIC_EnableIRQ(uint32_t irq);                 // Enable a specific IRQ
void APIC_DisableIRQ(uint32_t irq);                // Disable a specific IRQ

#endif // APIC_IRQ_H
