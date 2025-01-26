#include "terminal.h"
#include "timer.h"
#include "irq.h"
#include "pic.h"
#include "io.h"
#include <stdint.h>

#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40

uint32_t timer_ticks = 0;



void timer_set_pit_frequency(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency; // Base frequency is 1.193182 MHz
    // Configure PIT: Channel 0, Access mode: LSB/MSB, Mode 2 (rate generator), Binary
    outb(PIT_COMMAND, 0x36);

    // Send divisor: LSB first, then MSB
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));        // LSB
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF)); // MSB
}

void timer(Registers* regs) {
    ++timer_ticks;
}