#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "io.h"
#include "timer.h"
#include "hpet.h"
#include "atapi.h"
#include "memory.h"
#include "terminal.h"

#define ATAPI_SECTOR_SIZE 2048

// Register offsets relative to base port
#define ALTERNATE_STATUS 0
#define DATA 0
#define ERROR_R 1
#define SECTOR_COUNT 2
#define LBA_LOW 3
#define LBA_MID 4
#define LBA_HIGH 5
#define DRIVE_SELECT 6
#define COMMAND_REGISTER 7

uint16_t identify_data[256];

void delay_us(int microseconds) {
    pit_prepare_sleep(microseconds);
    pit_perform_sleep();
}

uint8_t read_status() {
    return inb(ATA_STATUS_REG);
}

int8_t wait_for_bsy_clear() {
    int timeout = 1000;
    while (read_status() & ATA_SR_BSY) {
        delay_us(100);
        if (--timeout == 0)
            return -1;
    }
    return 0;
}

int8_t wait_for_drq_set() {
    int timeout = 1000;
    while (!(read_status() & ATA_SR_DRQ)) {
        delay_us(100);
        if (--timeout == 0)
            return -1;
    }
    return 0;
}

void handle_error(const char *message) {
    terminal_printf("Error: %s\n", message);
    // Additional error handling if needed
}

int8_t ATAPI_Initialize() {
    outb(ATA_DEVICE_SELECT_REG, 0xA0); // master
    delay_us(1000); // 1ms delay

    outb(ATA_COMMAND_REG, ATA_IDENTIFY_PACKET_DEVICE);

    if (wait_for_bsy_clear() != 0) {
        terminal_printf("Error: No response to IDENTIFY PACKET DEVICE.\n");
        return -1;
    }
    if (wait_for_drq_set() != 0) {
        terminal_printf("Error: DRQ not set after IDENTIFY.\n");
        return -2;
    }
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(ATA_DATA_REG);
    }
    terminal_printf("ATAPI device inited successfully.\n");
    return 0;
}

// This code is to wait 400 ns
static void ata_io_wait(const uint8_t port) {
	inb(port + CONTROL + ALTERNATE_STATUS);
	inb(port + CONTROL + ALTERNATE_STATUS);
	inb(port + CONTROL + ALTERNATE_STATUS);
	inb(port + CONTROL + ALTERNATE_STATUS);
}

// Reads sectors starting from lba to buffer
int read_cdrom(uint16_t port, bool slave, uint32_t lba, uint32_t sectors, uint16_t *buffer) {
        // The command
	volatile uint8_t read_cmd[12] = {0xA8, 0,
	                                 (lba >> 0x18) & 0xFF, (lba >> 0x10) & 0xFF, (lba >> 0x08) & 0xFF,
	                                 (lba >> 0x00) & 0xFF,
	                                 (sectors >> 0x18) & 0xFF, (sectors >> 0x10) & 0xFF, (sectors >> 0x08) & 0xFF,
	                                 (sectors >> 0x00) & 0xFF,
	                                 0, 0};

	outb(port + DRIVE_SELECT, 0xA0 & (slave << 4)); // Drive select
	ata_io_wait(port);
	outb(port + ERROR_R, 0x00); 
	outb(port + LBA_MID, 2048 & 0xFF);
	outb(port + LBA_HIGH, 2048 >> 8);
	outb(port + COMMAND_REGISTER, 0xA0); // Packet command
	ata_io_wait(port); // I think we might need this delay, not sure, so keep this
 
        // Wait for status
	while (1) {
		uint8_t status = inb(port + COMMAND_REGISTER);
		if ((status & 0x01) == 1)
			return 1;
		if (!(status & 0x80) && (status & 0x08))
			break;
		ata_io_wait(port);
	}

        // Send command
	outsw(port + DATA, (uint16_t *) read_cmd, 6);

        // Read words
	for (uint32_t i = 0; i < sectors; i++) {
                // Wait until ready
		while (1) {
			uint8_t status = inb(port + COMMAND_REGISTER);
			if (status & 0x01)
				return 1;
			if (!(status & 0x80) && (status & 0x08))
				break;
		}

		int size = inb(port + LBA_HIGH) << 8
		           | inb(port + LBA_MID); // Get the size of transfer

		insw(port + DATA, (uint16_t *) ((uint8_t *) buffer + i * 0x800), size / 2); // Read it
	}

	return 0;
}