#include "terminal.h"
#include "acpi.h"
#include "apic.h"
#include "apic_irq.h"
#include "idt.h"
#include "isr.h"
#include "io.h"

uint32_t apic_base = APIC_BASE;
uint32_t apic_io_base = APIC_IO_BASE;

// Set the physical address for local APIC registers
void cpu_set_apic_base(uintptr_t apic) {
   uint32_t edx = 0;
   uint32_t eax = (apic & 0xFFFFF000) | IA32_APIC_BASE_MSR_ENABLE; // Ensure address is 4KB aligned

#ifdef __PHYSICAL_MEMORY_EXTENSION__
   edx = (apic >> 32) & 0x0F; // Handle 64-bit addressing
#endif

   cpuSetMSR(IA32_APIC_BASE_MSR, eax, edx);
}

// Get the physical address of the APIC registers page
uintptr_t cpu_get_apic_base() {
   uint32_t eax, edx;
   cpuGetMSR(IA32_APIC_BASE_MSR, &eax, &edx);

#ifdef __PHYSICAL_MEMORY_EXTENSION__
   return (eax & 0xFFFFF000) | ((edx & 0x0F) << 32);
#else
   return (eax & 0xFFFFF000);
#endif
}

// Configure IMCR to switch from PIC to APIC mode
void ConfigureIMCR() {
    uint8_t al = inb(0x22); // Read IMCR register
    if (al & 0x80) { // Check if IMCR is present
        outb(0x22, 0x70); // Select IMCR register
        outb(0x23, 0x1);  // Enable APIC mode
    }
}

// APIC Initialization function
void APIC_Initialize() {
    // Step 1: Ensure APIC is enabled in the IA32_APIC_BASE_MSR register
    uintptr_t apic_addr = cpu_get_apic_base();
    if (local_ioapic_address != 0) {
        apic_io_base = local_ioapic_address;
    }
    
    if (apic_addr == 0) {
        // If the APIC base address is 0, it means APIC is not enabled
        // Attempt to set the APIC base address
        cpu_set_apic_base(APIC_BASE);
        apic_addr = APIC_BASE;
    }

    cpu_set_apic_base(cpu_get_apic_base());
    uint32_t cr4 = ReadCR4();
    if (!(cr4 & CR4_APIC_BIT)) {
        WriteCR4(cr4|CR4_APIC_BIT);
        terminal_printf("APIC not enabled in CR4 register!\n");
    }

    // Step 2: Configure the IMCR (Interrupt Mode Configuration Register) to switch to APIC mode
    ConfigureIMCR();

    // Step 3: Set the Spurious Interrupt Vector Register (SVR)
    uint32_t svr_value = APIC_Read(APIC_SVR);
    svr_value &= ~0xFF;  // Clear existing vector
    svr_value |= 0x3F;    // Set to vector 0x0F (for spurious interrupts)
    svr_value |= 0x100;  // Set the APIC enable bit (bit 8)
    APIC_Write(APIC_SVR, svr_value);

    // Step 4: Set up the Local APIC ID Register (LID)
    uint32_t apic_id = APIC_Read(APIC_ID) >> 24; // Read APIC ID (bits 24-31)
    terminal_printf("APIC ID: 0x%x\n", apic_id);    
    terminal_printf("APIC Initialized\n");
}