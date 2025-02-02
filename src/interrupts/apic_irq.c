#include "terminal.h"
#include "pic_irq.h"
#include "apic.h"
#include "io.h"
#include <stddef.h>

#define APIC_REMAP_OFFSET        0x20  // Remap base for APIC interrupts
#define MAX_IRQS                 64    // Assuming up to 64 interrupts for simplicity

// APIC Register Offsets
#define APIC_ISR  0x100  // Interrupt Request Register (IRR)
#define APIC_IRR  0x200  // In-Service Register (ISR)

// Global IRQ Handlers
IRQHandler g_APICIRQHandlers[MAX_IRQS];

// Interrupt handler for APIC IRQs
void APIC_IRQ_Handler(Registers* regs) {
    int irq = regs->interrupt - APIC_REMAP_OFFSET;

    // Read the IRR (Interrupt Request Register) and ISR (In-Service Register) from the APIC
    uint32_t apic_isr = APIC_Read(APIC_ISR);  // APIC In-Service Register
    uint32_t apic_irr = APIC_Read(APIC_IRR);  // APIC Interrupt Request Register

    // Check if the IRQ has a registered handler
    if (g_APICIRQHandlers[irq] != NULL) {
        // Call the handler for the specific interrupt
        g_APICIRQHandlers[irq](regs);
    } else {
        terminal_printf("Unhandled APIC IRQ %d  ISR=%x  IRR=%x...\n", irq, apic_isr, apic_irr);
    }

    // Send EOI (End of Interrupt) to the APIC
    LAPIC_SendEOI();
}

// Initialize APIC IRQ handling
void APIC_IRQ_Initialize() {
    // Step 1: Initialize the Local APIC and configure for IRQs
    APIC_Initialize();

    // Step 2: Configure LVT entries for IRQs (e.g., LINT0, LINT1, Timer, etc.)
    // Mask the interrupt initially (set the mask bit to disable it)
    // APIC_Write(APIC_LVT_LINT0, APIC_DELIVERY_MODE_NMI | 0x10000); // Mask LINT0 (example)
    // APIC_Write(APIC_LVT_LINT1, APIC_DELIVERY_MODE_NMI | 0x10000); // Mask LINT1 (example)
    
    // Step 3: Register the IRQ handler for APIC IRQs
    for (int i = 0; i < MAX_IRQS; i++) {
        ISR_RegisterHandler(APIC_REMAP_OFFSET + i, APIC_IRQ_Handler);
    }

    terminal_printf("APIC IRQ Initialized\n");
}

// Register an IRQ handler for a specific APIC IRQ
void APIC_IRQ_RegisterHandler(uint32_t irq, IRQHandler handler) {
    if (irq < 0 || irq >= MAX_IRQS) {
        terminal_printf("Invalid IRQ number %d\n", irq);
        return;
    }

    g_APICIRQHandlers[irq] = handler;
}

// Set up a specific LVT entry (e.g., LINT0 or Timer) to enable interrupts
void APIC_EnableIRQ(uint32_t irq) {
    uint32_t value = APIC_Read(APIC_LVT_TIMER + irq * APIC_REGISTER_OFFSET);
    value &= ~0x10000; // Clear the mask bit (bit 16) to unmask the interrupt
    APIC_Write(APIC_LVT_TIMER + irq * APIC_REGISTER_OFFSET, value);
}

// Disable a specific IRQ
void APIC_DisableIRQ(uint32_t irq) {
    uint32_t value = APIC_Read(APIC_LVT_TIMER + irq * APIC_REGISTER_OFFSET);
    value |= 0x10000; // Set the mask bit (bit 16) to mask the interrupt
    APIC_Write(APIC_LVT_TIMER + irq * APIC_REGISTER_OFFSET, value);
}

  
// Enable I/O APIC interrupt (this is just a write to the redirection table entry)
void APIC_EnableIOIRQ(uint32_t irq, uint32_t interrupt_destination) {
    uint32_t redirection_entry = irq * APIC_REGISTER_OFFSET;

    // Set delivery mode to fixed, vector to the keyboard interrupt vector, unmask the interrupt
    uint32_t value = (interrupt_destination | APIC_DELIVERY_MODE_FIXED);
    value &= ~0x10000;  // Unmask interrupt (clear the mask bit)

    APIC_WriteIO(redirection_entry, value);
}

// Disable I/O APIC interrupt
void APIC_DisableIOIRQ(uint32_t irq) {
    uint32_t redirection_entry = irq * APIC_REGISTER_OFFSET;

    // Mask the interrupt by setting the mask bit
    uint32_t value = APIC_ReadIO(redirection_entry);
    value |= 0x10000;  // Mask interrupt (set the mask bit)
    APIC_WriteIO(redirection_entry, value);
}