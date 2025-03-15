#ifndef ATAPI_H
#define ATAPI_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define ATAPI_PACKET_SIZE 12
#define SECTOR_SIZE 2048  // El Torito sectors are typically 2048 bytes
#define TARGET_SECTOR 16  // Sector 16 contains the Primary Volume Descriptor
#define ATA_PRIMARY_BASE 0x1F0  // Base I/O port for primary ATA channel
#define ATA_COMMAND_REG (ATA_PRIMARY_BASE + 7)
#define ATA_STATUS_REG (ATA_PRIMARY_BASE + 7)
#define ATA_DATA_REG (ATA_PRIMARY_BASE)
#define ATA_DEVICE_SELECT_REG (ATA_PRIMARY_BASE + 6)
#define ATA_SECTOR_COUNT_REG (ATA_PRIMARY_BASE + 2)
#define ATA_LBA_LOW_REG (ATA_PRIMARY_BASE + 3)
#define ATA_LBA_MID_REG (ATA_PRIMARY_BASE + 4)
#define ATA_LBA_HIGH_REG (ATA_PRIMARY_BASE + 5)
#define ATA_CONTROL_REG 0x3F6  // Control register

#define ATA_IDENTIFY_PACKET_DEVICE 0xA1
#define ATA_SR_BSY 0x80
#define ATA_SR_DRQ 0x08
#define ATA_SR_ERR (ATA_PRIMARY_BASE + 1)
#define ATAPI_SECTOR_SIZE 2048

#define CONTROL 0x206

void handle_error(const char *message);
void read_atapi_sector_16();
int8_t ATAPI_Initialize();

static void ata_io_wait(const uint8_t port);
int read_cdrom(uint16_t port, bool slave, uint32_t lba, uint32_t sectors, uint16_t *buffer);

#endif // ATAPI_H