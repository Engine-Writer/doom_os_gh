#ifndef APIC_H
#define APIC_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char signature[4];  // E.g., 'DSDT', 'FACP'
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) acpi_header_t;

void acpi_parse_rsdt(acpi_header_t *rsdt_data);

#endif // APIC_H