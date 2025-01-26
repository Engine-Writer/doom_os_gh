#include "multiboot2.h"
#include "acpi.h"
#include "terminal.h"
#include "memory.h"
#include "util.h"

void acpi_parse_rsdt(acpi_header_t *rsdt_data) {
    acpi_header_t *header = (acpi_header_t *)rsdt_data;
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
    uint32_t *table_pointers = (uint32_t *)((char *)rsdt_data + sizeof(acpi_header_t));

    terminal_printf("Found %d ACPI tables in RSDT\n", num_tables);

    // Process each table pointer
    for (uint32_t i = 0; i < num_tables; ++i) {
        uint32_t table_address = table_pointers[i];  // Fixed pointer access
        terminal_printf("Table %d: Address=0x%x\n", i, table_address);

        // You can now fetch the table at this address and parse it
        // For example, check the signature and handle accordingly
        acpi_header_t *table_header = (acpi_header_t *)table_address;

        if (memcmp(table_header->signature, "DSDT", 4) == 0) {
            terminal_printf("Found DSDT table\n");
            // Process DSDT
        } else if (memcmp(table_header->signature, "MADT", 4) == 0) {
            terminal_printf("Found MADT table\n");
            // Process MADT
        } else if (memcmp(table_header->signature, "FACP", 4) == 0) {
            terminal_printf("Found FACP table\n");
            // Process FACP
        } else if (memcmp(table_header->signature, "APIC", 4) == 0) {
            terminal_printf("Found APIC table\n");
            // Process APIC
        } else if (memcmp(table_header->signature, "HPET", 4) == 0) {
            terminal_printf("Found HPET table\n");
            // Process HPET
        } else if (memcmp(table_header->signature, "MCFG", 4) == 0) {
            terminal_printf("Found MCFG table\n");
            // Process MCFG
        } else if (memcmp(table_header->signature, "SSDT", 4) == 0) {
            terminal_printf("Found SSDT table\n");
            // Process SSDT
        } else {
            memcpy(charcharx, table_header->signature, 4);            
            charcharx[4] = '\0';  // Null-terminate the string
            terminal_printf("Unknown ACPI table: %s\n", charcharx);
        }
    }
}