#include "sound.h"
#include "io.h"
#include <stdint.h>

#define PIT_COMMAND 0x43
#define PIT_CHANNEL2 0x42
#define SPEAKER_CTRL 0x61

void sound_play_frequency(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;

    outb(PIT_COMMAND, 0xB6);                     // Set PIT Channel 2 in square wave mode
    outb(PIT_CHANNEL2, (uint8_t)(divisor & 0xFF)); // Send LSB
    outb(PIT_CHANNEL2, (uint8_t)((divisor >> 8) & 0xFF)); // Send MSB

    uint8_t tmp = inb(SPEAKER_CTRL);
    if (tmp != (tmp | 3)) {
 		outb(SPEAKER_CTRL, tmp | 3);
 	}
}

void sound_stop() {
    uint8_t tmp = inb(SPEAKER_CTRL);
    outb(SPEAKER_CTRL, tmp & ~3);               // Disable speaker
}