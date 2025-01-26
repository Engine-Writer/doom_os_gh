#include "terminal.h"
#include "timer.h"
#include "irq.h"
#include "pic.h"
#include "io.h"
#include <stdint.h>

#define TIMER_TPS 125
#define PIT_HZ 1193181
#define DIV_OF_FREQ(_f) (PIT_HZ / (_f))
#define FREQ_OF_DIV(_d) (PIT_HZ / (_d))
#define REAL_FREQ_OF_FREQ(_f) (FREQ_OF_DIV(DIV_OF_FREQ((_f))))

#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40

uint32_t timer_ticks = 0;
uint32_t timer_frequency = 0;
uint32_t timer_divisor = 0;


void timer_set_pit_frequency(uint32_t frequency) {
    if (frequency == 0) {
        return; // Avoid divide-by-zero error
    }

    uint32_t divisor = 1193131.666 / frequency; // Base frequency is 1.193182 MHz

    // Ensure divisor is within the 16-bit range (0-65535)
    if (divisor < 1) divisor = 1;      // Avoid too low frequency
    if (divisor > 65535) divisor = 65535; // Cap to max value for PIT

    // Configure PIT: Channel 0, Access mode: LSB/MSB, Mode 2 (rate generator), Binary
    outb(PIT_COMMAND, 0x36);
    iowait();

    // Send divisor: LSB first, then MSB
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));        // LSB
    iowait();
    outb(PIT_CHANNEL0, (uint8_t)((divisor & 0xFF00) >> 8)); // MSB
    iowait();
}


void timer_handler(Registers* regs) {
    ++timer_ticks;
}

void timer_init() {
    const uint32_t freq = REAL_FREQ_OF_FREQ(TIMER_TPS);
    timer_frequency = freq;
    timer_divisor = DIV_OF_FREQ(freq);
    timer_ticks = 0;
    timer_set_pit_frequency(TIMER_TPS);
    IRQ_RegisterHandler(0, (IRQHandler)timer_handler);
}