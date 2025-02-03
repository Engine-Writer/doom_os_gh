#ifndef ACPI_H
#define ACPI_H

#include <stdint.h>
#include <stddef.h>
#include "isr.h"


// Define the sleep type constants for each state
#define SLEEP_STATE_S0 0
#define SLEEP_STATE_S1 1
#define SLEEP_STATE_S2 2
#define SLEEP_STATE_S3 3
#define SLEEP_STATE_S4 4
#define SLEEP_STATE_S5 5

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

typedef struct {
    acpi_header_t acpi_header;    // Standard ACPI table header
    uint32_t rsdt_tags[];           // Pointer to ACPI data (methods, device objects, etc.)
} __attribute__((packed)) rsdt_table_t;

// FACP Table

typedef struct {
  uint8_t AddressSpace;
  uint8_t BitWidth;
  uint8_t BitOffset;
  uint8_t AccessSize;
  uint64_t Address;
} GenericAddressStructure_t;


typedef struct {
    acpi_header_t header;       // ACPI table header
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;

    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8_t  Reserved;

    uint8_t  PreferredPowerManagementProfile;
    uint16_t SCI_Interrupt;
    uint32_t SMI_CommandPort;
    uint8_t  AcpiEnable;
    uint8_t  AcpiDisable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t  PM1EventLength;
    uint8_t  PM1ControlLength;
    uint8_t  PM2ControlLength;
    uint8_t  PMTimerLength;
    uint8_t  GPE0Length;
    uint8_t  GPE1Length;
    uint8_t  GPE1Base;
    uint8_t  CStateControl;
    uint16_t WorstC2Latency;
    uint16_t WorstC3Latency;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t  DutyOffset;
    uint8_t  DutyWidth;
    uint8_t  DayAlarm;
    uint8_t  MonthAlarm;
    uint8_t  Century;

    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t BootArchitectureFlags;

    uint8_t  Reserved2;
    uint32_t Flags;

    // 12 byte structure; see below for details
    GenericAddressStructure_t ResetReg;

    uint8_t  ResetValue;
    uint8_t  Reserved3[3];
  
    // 64bit pointers - Available on ACPI 2.0+
    uint64_t                X_FirmwareControl;
    uint64_t                X_Dsdt;

    GenericAddressStructure_t X_PM1aEventBlock;
    GenericAddressStructure_t X_PM1bEventBlock;
    GenericAddressStructure_t X_PM1aControlBlock;
    GenericAddressStructure_t X_PM1bControlBlock;
    GenericAddressStructure_t X_PM2ControlBlock;
    GenericAddressStructure_t X_PMTimerBlock;
    GenericAddressStructure_t X_GPE0Block;
    GenericAddressStructure_t X_GPE1Block;
} __attribute__((packed)) facp_table_t;
typedef struct {
    acpi_header_t acpi_header;    // Standard ACPI table header
    uint32_t reserved;            // Reserved field
    uint8_t *acpi_data;           // Pointer to ACPI data (methods, device objects, etc.)
} __attribute__((packed)) dsdt_table_t;

// APIC Table

// Processor Local APIC Entry (Entry Type 0)
typedef struct {
    uint8_t type;            // Entry type (0 = Processor Local APIC)
    uint8_t length;          // Length of the entry (8 bytes for Processor Local APIC)
    uint8_t acpi_processor_id;  // ACPI Processor ID
    uint8_t apic_id;         // APIC ID
    uint32_t flags;          // Flags (Processor Enabled, Online Capable)
} __attribute__((packed)) processor_local_apic_entry_t;

// I/O APIC Entry (Entry Type 1)
typedef struct {
    uint8_t type;            // Entry type (1 = I/O APIC)
    uint8_t length;          // Length of the entry (12 bytes for I/O APIC)
    uint8_t apic_id;         // I/O APIC's ID
    uint8_t reserved;        // Reserved (0)
    uint32_t ioapic_address; // Physical address of the I/O APIC
    uint32_t global_irq_base;// Global System Interrupt Base
} __attribute__((packed)) ioapic_entry_t;

// I/O APIC Interrupt Source Override Entry (Entry Type 2)
typedef struct {
    uint8_t type;             // Entry type (2 = I/O APIC Interrupt Source Override)
    uint8_t length;           // Length of the entry (10 bytes for Interrupt Source Override)
    uint8_t bus_source;       // Bus Source
    uint8_t irq_source;       // IRQ Source
    uint32_t global_irq;      // Global System Interrupt
    uint16_t flags;           // Flags (reserved bits)
} __attribute__((packed)) ioapic_interrupt_source_override_entry_t;

// I/O APIC Non-maskable interrupt source Entry (Entry Type 3)
typedef struct {
    uint8_t type;             // Entry type (3 = I/O APIC Non-maskable interrupt)
    uint8_t length;           // Length of the entry (10 bytes for Non-maskable interrupt)
    uint8_t nmi_source;       // NMI Source
    uint8_t reserved;         // Reserved
    uint16_t flags;           // Flags (reserved bits)
    uint32_t global_irq;      // Global System Interrupt
} __attribute__((packed)) ioapic_nmi_source_entry_t;

// Local APIC Non-maskable interrupts Entry (Entry Type 4)
typedef struct {
    uint8_t type;             // Entry type (4 = Local APIC Non-maskable interrupts)
    uint8_t length;           // Length of the entry (8 bytes)
    uint8_t acpi_processor_id; // ACPI Processor ID
    uint16_t flags;           // Flags
    uint8_t lint;             // LINT# (0 or 1)
} __attribute__((packed)) local_apic_nmi_entry_t;

// Local APIC Address Override Entry (Entry Type 5)
typedef struct {
    uint8_t type;             // Entry type (5 = Local APIC Address Override)
    uint8_t length;           // Length of the entry (10 bytes)
    uint8_t reserved[2];      // Reserved
    uint64_t local_apic_address; // 64-bit physical address of Local APIC
} __attribute__((packed)) local_apic_address_override_entry_t;

// Processor Local x2APIC Entry (Entry Type 9)
typedef struct {
    uint8_t type;             // Entry type (9 = Processor Local x2APIC)
    uint8_t length;           // Length of the entry (16 bytes)
    uint16_t reserved;        // Reserved
    uint32_t processor_x2apic_id; // Processor's local x2APIC ID
    uint32_t flags;           // Flags (same as the Local APIC flags)
    uint32_t acpi_id;         // ACPI Processor ID
} __attribute__((packed)) processor_local_x2apic_entry_t;

// Union of all possible entry types in the MADT
typedef union {
    processor_local_apic_entry_t processor_local_apic;
    ioapic_entry_t ioapic;
    ioapic_interrupt_source_override_entry_t ioapic_interrupt_source_override;
    ioapic_nmi_source_entry_t ioapic_nmi_source;
    local_apic_nmi_entry_t local_apic_nmi;
    local_apic_address_override_entry_t local_apic_address_override;
    processor_local_x2apic_entry_t processor_local_x2apic;
} __attribute__((packed)) apic_entry_t;

typedef struct {
    acpi_header_t header;       // ACPI table header
    uint32_t local_apic_address; // Address of the local APIC
    uint32_t flags;             // Flags describing the system
    uint8_t apic_entries[];     // APIC entries (specific to the platform)
} __attribute__((packed)) madt_table_t;

// HPET Table
typedef struct {
    acpi_header_t header;       // ACPI table header
    uint32_t hpet_address;      // Address of the HPET
    uint8_t min_clock_tick;     // Minimum clock tick in 100ns units
    uint8_t page_protection;    // Protection flags
    uint16_t reserved;          // Reserved field
} __attribute__((packed)) hpet_table_t;

// MCFG Table
typedef struct {
    uint64_t base_address;      // Offset 0: Base address of enhanced configuration mechanism
    uint16_t segment_group;     // Offset 8: PCI Segment Group Number
    uint8_t bus_start;          // Offset 10: Start PCI bus number
    uint8_t bus_end;            // Offset 11: End PCI bus number
    uint32_t reserved;          // Offset 12: Reserved
} __attribute__((packed)) mcfg_allocation_t;

typedef struct {
    acpi_header_t header;       // ACPI common header
    uint64_t reserved;          // Offset 36: Reserved field
    mcfg_allocation_t allocations[]; // Offset 44: Array of allocation entries
} __attribute__((packed)) mcfg_table_t;


// SSDT Table
typedef struct {
    acpi_header_t header;      // ACPI table header
    uint32_t reserved;         // Reserved field
    uint8_t ssd_entries[];     // SSDT entries (specific to the platform)
} __attribute__((packed)) ssdt_table_t;


// ACPI-related variables
extern uint32_t firmware_ctrl;
extern uint32_t dsdt_address;
extern uint16_t sci_int;
extern uint64_t pm_timer_addr;

extern uint32_t  local_apic_address;
extern uint32_t local_ioapic_address;
extern uint32_t  apic_flags;
extern uint32_t *apic_entries;

extern uint32_t hpet_address;
extern uint8_t  min_clock_tick;

extern uint32_t SLP_TYP0a;
extern uint32_t SLP_TYP1a;
extern uint32_t SLP_TYP2a;
extern uint32_t SLP_TYP3a;
extern uint32_t SLP_TYP4a;
extern uint32_t SLP_TYP5a;
extern uint32_t SLP_TYP0b;
extern uint32_t SLP_TYP1b;
extern uint32_t SLP_TYP2b;
extern uint32_t SLP_TYP3b;
extern uint32_t SLP_TYP4b;
extern uint32_t SLP_TYP5b;

extern uint32_t acpi_PM1aControlBlock;
extern uint32_t acpi_PM1bControlBlock;

extern uint64_t acpi_rrap;
extern uint8_t  acpi_rval;
extern uint8_t *S4BIOS_REQ;

extern mcfg_allocation_t *PCIe_data;
extern hpet_table_t *hpet_data;
extern acpi_header_t *rsdt_data;

extern uint8_t *ssdt_entries;

extern facp_table_t *facp_table_gb;  // Initialized by parsing RSDT and finding FACP table


// Function Prototypes
void acpi_parse_dsdt(dsdt_table_t *dsdt_table);
void acpi_parse_facp(facp_table_t *facp_table);
void acpi_parse_apic(madt_table_t *apic_table);
void acpi_parse_hpet(hpet_table_t *hpet_table);
void acpi_parse_mcfg(mcfg_table_t *mcfg_table);
void acpi_parse_ssdt(ssdt_table_t *ssdt_table);
void acpi_parse_rsdt(acpi_header_t *rsdt_table);
void acpi_init(acpi_header_t *rsdt_table);
void acpi_sci_handler(Registers *regs);
void sci_apic_init();
void IOAPIC_ConfigureSCI();
void acpi_setup_event_enables();

void ACPI_ENABLE();
void ACPI_DISABLE();

#endif // ACPI_H