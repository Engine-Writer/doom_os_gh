#include "acpi.h"
#include "terminal.h"
#include "memory.h"
#include "idt.h"
#include "isr.h"
#include "pic_irq.h"
#include "apic_irq.h"
#include "pic.h"
#include "apic.h"
#include "util.h"
#include "io.h"


// ACPI-related variables (defined here)
uint32_t firmware_ctrl;
uint32_t dsdt_address;
uint16_t sci_int;
uint64_t pm_timer_addr;

uint32_t local_apic_address;
uint32_t local_ioapic_address;
uint32_t apic_flags;
uint32_t *apic_entries;

uint32_t hpet_address;
uint8_t min_clock_tick;

uint32_t SLP_TYP0a;
uint32_t SLP_TYP1a;
uint32_t SLP_TYP2a;
uint32_t SLP_TYP3a;
uint32_t SLP_TYP4a;
uint32_t SLP_TYP5a;
uint32_t SLP_TYP0b;
uint32_t SLP_TYP1b;
uint32_t SLP_TYP2b;
uint32_t SLP_TYP3b;
uint32_t SLP_TYP4b;
uint32_t SLP_TYP5b;

uint32_t acpi_PM1aControlBlock;
uint32_t acpi_PM1bControlBlock;

uint64_t acpi_rrap;
uint8_t  acpi_rval;
uint8_t *S4BIOS_REQ;

mcfg_allocation_t *PCIe_data;
hpet_table_t      *hpet_data;
acpi_header_t     *rsdt_data;

uint8_t *ssdt_entries;

// Pointer to FACP (Firmware Control)
facp_table_t *facp_table_gb;  // This should be initialized by parsing the RSDT and finding the FACP table

// Function to parse the DSDT table and extract sleep states
void acpi_parse_dsdt(dsdt_table_t *dsdt_table) {
    // Allocate memory for ACPI data
    uint32_t acpi_data_length = dsdt_table->acpi_header.length - sizeof(dsdt_table_t);
    dsdt_table->acpi_data = (uint8_t *)memalloc(acpi_data_length);
    uintptr_t *analyze_addr = (uintptr_t *)(dsdt_table->acpi_data);

    // Copy the ACPI data from the DSDT table
    memcpy(dsdt_table->acpi_data, (uint8_t *)dsdt_table + sizeof(dsdt_table_t), acpi_data_length);

    // Parse the sleep states and extract corresponding SLP_TYPx values
    for (int state = SLEEP_STATE_S0; state <= SLEEP_STATE_S5; state++) {
        char method_name[5] = { '_', 'S', '0' + state, '_', '\0' }; // Create method names: _S0_, _S1_, ...
        
        // Search for the sleep state method (e.g., _S0_, _S1_, etc.)
        uint8_t *method_addr = (uint8_t *)analyze_addr;
        uint32_t remaining_length = acpi_data_length;
        while (remaining_length--) {
            if (memcmp((char *)method_addr, method_name, 4) == 0)
                break;
            method_addr++;
        }

        // If method is found, parse it
        if (remaining_length > 0) {
            // Validate the AML structure (simple validation for this example)
            if ((*(method_addr - 1) == 0x08 || (*(method_addr - 2) == 0x08 && *(method_addr - 1) == '\\')) && *(method_addr + 4) == 0x12) {
                method_addr += 5;
                method_addr += ((*method_addr & 0xC0) >> 6) + 2;  // Calculate PkgLength size

                // Extract sleep type (SLP_TYPx) for the given state
                uint16_t sleep_type = *(method_addr) << 10;
                if (*method_addr == 0x0A) {
                    uint16_t sleep_type = *(method_addr+1) << 10;
                }

                // Log the sleep state and its corresponding SLP_TYPx value
                terminal_printf("Found %s with sleep type: 0x%x\n", method_name, sleep_type);
                
                // Store the sleep types if necessary, depending on your system
                switch (state) {
                    case SLEEP_STATE_S0:
                        if (*method_addr == 0x0A)
                            method_addr++;	// skip byteprefix
                        SLP_TYP0a = *(method_addr)<<10;
                        method_addr++;

                        if (*method_addr == 0x0A)
                            method_addr++;	// skip byteprefix
                        SLP_TYP0b = *(method_addr)<<10;
                        break;
                    case SLEEP_STATE_S1:
                        if (*method_addr == 0x0A)
                            method_addr++;	// skip byteprefix
                        SLP_TYP1a = *(method_addr)<<10;
                        method_addr++;

                        if (*method_addr == 0x0A)
                            method_addr++;	// skip byteprefix
                        SLP_TYP1b = *(method_addr)<<10;
                        break;
                    case SLEEP_STATE_S2:
                        if (*method_addr == 0x0A)
                            method_addr++;	// skip byteprefix
                        SLP_TYP2a = *(method_addr)<<10;
                        method_addr++;

                        if (*method_addr == 0x0A)
                            method_addr++;	// skip byteprefix
                        SLP_TYP2b = *(method_addr)<<10;
                        break;
                    case SLEEP_STATE_S3:
                        if (*method_addr == 0x0A)
                            method_addr++;	// skip byteprefix
                        SLP_TYP3a = *(method_addr)<<10;
                        method_addr++;

                        if (*method_addr == 0x0A)
                            method_addr++;	// skip byteprefix
                        SLP_TYP3b = *(method_addr)<<10;
                        break;
                    case SLEEP_STATE_S4:
                        if (*method_addr == 0x0A)
                            method_addr++;	// skip byteprefix
                        SLP_TYP4a = *(method_addr)<<10;
                        method_addr++;

                        if (*method_addr == 0x0A)
                            method_addr++;	// skip byteprefix
                        SLP_TYP4b = *(method_addr)<<10;
                        break;
                    case SLEEP_STATE_S5:
                        if (*method_addr == 0x0A)
                            method_addr++;	// skip byteprefix
                        SLP_TYP5a = *(method_addr)<<10;
                        method_addr++;

                        if (*method_addr == 0x0A)
                            method_addr++;	// skip byteprefix
                        SLP_TYP5b = *(method_addr)<<10;
                        break;
                    default:
                        break;
                }
            }
        } else {
            terminal_printf("Method %s not found in the DSDT table.\n", method_name);
        }
    }
}

// Process the FACP table
void acpi_parse_facp(facp_table_t *facp_table) {
    firmware_ctrl = facp_table->FirmwareCtrl;
    dsdt_address = facp_table->Dsdt;
    sci_int = facp_table->SCI_Interrupt;
    pm_timer_addr = facp_table->PMTimerBlock;
    acpi_PM1aControlBlock = facp_table->PM1aControlBlock;
    acpi_PM1bControlBlock = facp_table->PM1bControlBlock;

    facp_table_gb = facp_table;
    // check if acpi is enabled
	if ((inw((uint16_t)(facp_table->PM1aControlBlock&0x0000FFFF))&sci_int) == 0) {
		// check if acpi can be enabled
		if (facp_table->SMI_CommandPort != 0 && facp_table->AcpiEnable != 0) {
			outb((uint16_t)(facp_table->SMI_CommandPort&0x0000FFFF), facp_table->AcpiEnable); // send acpi enable command
        }
    }

    if (memcmp((char *)dsdt_address, "DSDT", 4) == 0) {
        acpi_parse_dsdt((dsdt_table_t *)dsdt_address);
    }
    acpi_rrap = facp_table->ResetReg.Address;
    acpi_rval = facp_table->ResetValue;

    S4BIOS_REQ = (uint8_t *)&facp_table->S4BIOS_REQ;
    
    terminal_printf("Firmware Control Address: 0x%x\n", firmware_ctrl);
    terminal_printf("DSDT Address: 0x%x\n", dsdt_address);
    terminal_printf("SCI Interrupt Number: %d\n", sci_int);
    terminal_printf("PM Timer Address: 0x%x\n", pm_timer_addr);
}

// Process the APIC table
void acpi_parse_madt(madt_table_t *madt_table) {
    // Step 1: Store Local APIC Address and Flags
    local_apic_address = madt_table->local_apic_address;
    apic_flags = madt_table->flags;
    apic_base = madt_table->local_apic_address;

    // Print basic APIC information
    terminal_printf("Local APIC Address: 0x%x\n", local_apic_address);
    terminal_printf("Flags: 0x%x\n", apic_flags);

    // Step 2: Get the start and end of the APIC entries array
    uint8_t *entry_ptr = madt_table->apic_entries;
    uint8_t *end_ptr = (uint8_t *)madt_table + madt_table->header.length;

    // Step 3: Process each APIC entry
    while (entry_ptr < end_ptr) {
        uint8_t type = entry_ptr[0];   // First byte is the entry type
        uint8_t length = entry_ptr[1]; // Second byte is the entry length

        // If length is invalid (to avoid infinite loops)
        if (length < 2) {
            terminal_printf("Error: Invalid MADT entry length %d\n", length);
            break;
        }

        switch (type) {
            case 0: { // Processor Local APIC
                processor_local_apic_entry_t *entry = (processor_local_apic_entry_t *)entry_ptr;
                terminal_printf("CPU APIC: ID=%d, ACPI_ID=%d, Flags=0x%x\n",
                                entry->apic_id, entry->acpi_processor_id, entry->flags);
                break;
            }
            
            case 1: { // I/O APIC
                ioapic_entry_t *entry = (ioapic_entry_t *)entry_ptr;
                terminal_printf("I/O APIC: ID=%d, Address=0x%x, Global IRQ Base=%d\n",
                                entry->apic_id, entry->ioapic_address, entry->global_irq_base);
                local_ioapic_address = entry->ioapic_address;
                break;
            }

            case 2: { // Interrupt Source Override
                ioapic_interrupt_source_override_entry_t *entry = (ioapic_interrupt_source_override_entry_t *)entry_ptr;
                terminal_printf("IRQ Override: Bus Source=%d, IRQ Source=%d, Global IRQ=%d, Flags=0x%x\n",
                                entry->bus_source, entry->irq_source, entry->global_irq, entry->flags);
                break;
            }

            case 3: { // Non-maskable Interrupt Source
                ioapic_nmi_source_entry_t *entry = (ioapic_nmi_source_entry_t *)entry_ptr;
                terminal_printf("I/O APIC NMI: Source=%d, Global IRQ=%d, Flags=0x%x\n",
                                entry->nmi_source, entry->global_irq, entry->flags);
                break;
            }

            case 4: { // Local APIC Non-maskable Interrupt
                local_apic_nmi_entry_t *entry = (local_apic_nmi_entry_t *)entry_ptr;
                terminal_printf("Local APIC NMI: Processor ID=%d, LINT=%d, Flags=0x%x\n",
                                entry->acpi_processor_id, entry->lint, entry->flags);
                break;
            }

            case 5: { // Local APIC Address Override (for 64-bit systems)
                local_apic_address_override_entry_t *entry = (local_apic_address_override_entry_t *)entry_ptr;
                terminal_printf("Local APIC Address Override: New Address=0x%lx\n", entry->local_apic_address);
                local_apic_address = entry->local_apic_address; // Update address
                break;
            }

            case 9: { // Processor Local x2APIC (for newer systems)
                processor_local_x2apic_entry_t *entry = (processor_local_x2apic_entry_t *)entry_ptr;
                terminal_printf("CPU x2APIC: ID=%d, ACPI_ID=%d, Flags=0x%x\n",
                                entry->processor_x2apic_id, entry->acpi_id, entry->flags);
                break;
            }

            default: // Unknown or unhandled APIC entry type
                terminal_printf("Unknown APIC Entry: Type=%d, Length=%d\n", type, length);
                break;
        }

        // Move to the next APIC entry
        entry_ptr += length;
    }

    terminal_printf("MADT Parsing Completed.\n");
}


// Process the HPET table
void acpi_parse_hpet(hpet_table_t *hpet_table) {
    hpet_address = hpet_table->hpet_address;
    min_clock_tick = hpet_table->min_clock_tick;

    hpet_data = hpet_table;

    terminal_printf("HPET Address: 0x%x\n", hpet_address);
    terminal_printf("Minimum Clock Tick: %d ns\n", min_clock_tick);
}

// Process the MCFG table
void acpi_parse_mcfg(mcfg_table_t *mcfg_table) {
    uint32_t allocation_count = (mcfg_table->header.length - sizeof(acpi_header_t) - sizeof(uint64_t)) / sizeof(mcfg_allocation_t);

    PCIe_data = (mcfg_allocation_t *)memalloc(allocation_count * sizeof(mcfg_allocation_t));
    if (!PCIe_data) {
        terminal_printf("Failed to allocate memory for PCIe configuration entries.\n");
        return;
    }

    // Copy the entries from the table to the allocated memory
    memcpy(PCIe_data, mcfg_table->allocations, allocation_count * sizeof(mcfg_allocation_t));
}

// Process the SSDT table
void acpi_parse_ssdt(ssdt_table_t *ssdt_table) {
    // Dynamically allocate memory for SSDT entries
    ssdt_entries = (uint8_t *)memalloc(ssdt_table->header.length - sizeof(acpi_header_t));

    // Copy the SSDT entries from the table
    memcpy(ssdt_entries, ssdt_table->ssd_entries, ssdt_table->header.length - sizeof(acpi_header_t));

    terminal_printf("SSDT Table size %d\n", ssdt_table->header.length - sizeof(acpi_header_t));
    uint8_t *entry = ssdt_entries;
    while (entry < (uint8_t *)ssdt_table + ssdt_table->header.length) {
        // Parse each SSDT entry
        entry++;  // Move to the next entry
    }
}

// Main RSDT parsing function
void acpi_parse_rsdt(acpi_header_t *rsdt_table) {
    acpi_header_t *header = (acpi_header_t *)rsdt_table;
    char charcharx[5];

    // Verify the signature
    if (memcmp(header->signature, "RSDT", 4) != 0) {
        memcpy(charcharx, header->signature, 4);            
        charcharx[4] = '\0';  // Null-terminate the string
        terminal_printf("Invalid RSDT signature: %s\n", charcharx);
        return;
    }

    // Extract the list of table pointers
    uint32_t num_tables = (header->length - sizeof(acpi_header_t)) / sizeof(uint32_t);
    uint32_t *table_pointers = (uint32_t *)((char *)rsdt_table + sizeof(acpi_header_t));

    terminal_printf("Found %d ACPI tables in RSDT\n", num_tables);

    // Process each table pointer
    for (uint32_t i = 0; i < num_tables; ++i) {
        uint32_t table_address = table_pointers[i];
        // terminal_printf("Table %d: Address=0x%x\n", i, table_address);

        acpi_header_t *table_header = (acpi_header_t *)table_address;

        if (memcmp(table_header->signature, "DSDT", 4) == 0) {
            terminal_printf("Found DSDT table %d, Address=0x%x\n", i, table_address);
        } else if (memcmp(table_header->signature, "FACP", 4) == 0) {
            terminal_printf("Found FACP table %d, Address=0x%x\n", i, table_address);
            facp_table_t *facp_table = (facp_table_t *)table_address;
            acpi_parse_facp(facp_table);
        } else if (memcmp(table_header->signature, "APIC", 4) == 0) {
            terminal_printf("Found MADT table %d, Address=0x%x\n", i, table_address);
            madt_table_t *apic_table = (madt_table_t *)table_address;
            acpi_parse_madt(apic_table);
        } else if (memcmp(table_header->signature, "HPET", 4) == 0) {
            terminal_printf("Found HPET table %d, Address=0x%x\n", i, table_address);
            hpet_table_t *hpet_table = (hpet_table_t *)table_address;
            acpi_parse_hpet(hpet_table);
        } else if (memcmp(table_header->signature, "MCFG", 4) == 0) {
            terminal_printf("Found MCFG table %d, Address=0x%x\n", i, table_address);
            mcfg_table_t *mcfg_table = (mcfg_table_t *)table_address;
            acpi_parse_mcfg(mcfg_table);
        } else if (memcmp(table_header->signature, "SSDT", 4) == 0) {
            terminal_printf("Found SSDT table %d, Address=0x%x\n", i, table_address);
            ssdt_table_t *ssdt_table = (ssdt_table_t *)table_address;
            acpi_parse_ssdt(ssdt_table);
        } else {
            memcpy(charcharx, table_header->signature, 4);            
            charcharx[4] = '\0';  // Null-terminate the string
            terminal_printf("Unknown ACPI table: %s\n", charcharx);
        }
    }
}

void acpi_init(acpi_header_t *rsdt_table) {
    acpi_parse_rsdt(rsdt_table);
    rsdt_data = rsdt_table;
}

void acpi_sci_handler(Registers *regs) {
    terminal_writestring("SCI Interrupt!\n");
    // APIC_SendEOI();
}

void acpiPowerOff(void) {
    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);
    outw(0x4004, 0x3400);

	// SCI_EN is set to 1 if acpi shutdown is possible
	if (sci_int == 0)
		return;

	// send the shutdown command
	outw((uint16_t)acpi_PM1aControlBlock, SLP_TYP5a | 1<<13 );
	if (acpi_PM1bControlBlock != 0)
		outw((uint16_t)acpi_PM1bControlBlock, SLP_TYP5b | 1<<13 );
}

void acpiReboot() {
    outb(acpi_rrap, acpi_rval);

    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    HALT();
}