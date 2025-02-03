#include "terminal.h"
#include "timer.h"
#include "pic_irq.h"
#include "pic.h"
#include "apic_irq.h"
#include "apic.h"
#include "io.h"
#include <stdint.h>

// Timer and frequency definitions
#define TIMER_TPS 125
#define PIT_HZ 1193181
#define DIV_OF_FREQ(_f) (PIT_HZ / (_f))
#define FREQ_OF_DIV(_d) (PIT_HZ / (_d))
#define REAL_FREQ_OF_FREQ(_f) (FREQ_OF_DIV(DIV_OF_FREQ((_f))))

// PIT ports and commands
#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40

// APIC Timer registers
#define APIC_TIMER_INIT_COUNT 0xFFFFFFFF  // Max initial count
#define APIC_TIMER_PERIODIC   0x20000     // Periodic mode
#define APIC_TIMER_DIVIDE_64  0x06        // Divide by 64
#define APIC_TIMER_IRQ_VECTOR 0x20        // IRQ vector
#define APIC_TIMER_INITCNT    0x380       // Initial counter
#define APIC_TIMER_CURRCNT    0x390       // Current counter
#define APIC_TIMER_DIV        0x3E0       // Timer divide configuration

// Timer state variables
uint32_t timer_ticks = 0;
uint32_t timer_frequency = 0;
uint32_t timer_divisor = 0;

void timer_pic_set_pit_frequency(uint32_t frequency) {
    if (frequency == 0) {
        return; // Avoid division by zero
    }

    uint32_t divisor = (PIT_HZ + (frequency / 2)) / frequency; // Round properly

    // Ensure divisor is within valid 16-bit range (1-65535)
    if (divisor < 1) divisor = 1;
    if (divisor > 65535) divisor = 65535;

    // Configure PIT: Channel 0, Access mode: LSB/MSB, Mode 2 (rate generator), Binary
    outb(PIT_COMMAND, 0x36);
    iowait();

    // Send divisor: LSB first, then MSB
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF)); // LSB
    iowait();
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF)); // MSB
    iowait();
}

void timer_handler(Registers* regs) {
    (void)regs;  // Prevent unused parameter warning
    ++timer_ticks;
}

void timer_pic_init() {
    const uint32_t freq = REAL_FREQ_OF_FREQ(TIMER_TPS);
    timer_frequency = freq;
    timer_divisor = DIV_OF_FREQ(freq);
    timer_ticks = 0;
    timer_pic_set_pit_frequency(TIMER_TPS);
    PIC_IRQ_RegisterHandler(0, (IRQHandler)timer_handler);
}

void timer_apic_init() {
    // Set APIC timer divider to 64
    APIC_Write(APIC_TIMER_DIV, APIC_TIMER_DIVIDE_64);

    // Prepare the PIT for a 10ms sleep (10000µs)
    pit_prepare_sleep(10000);

    // Start APIC timer with max initial count
    APIC_Write(APIC_TIMER_INITCNT, APIC_TIMER_INIT_COUNT);
    APIC_Write(APIC_LVT_TIMER, APIC_Read(APIC_LVT_TIMER) & ~APIC_LVT_INT_MASKED);

    // Perform PIT-based delay
    pit_perform_sleep();

    // Stop the APIC timer
    APIC_Write(APIC_LVT_TIMER, APIC_LVT_INT_MASKED);

    // Calculate APIC ticks per 10ms
    uint32_t ticksIn10ms = APIC_TIMER_INIT_COUNT - APIC_Read(APIC_TIMER_CURRCNT);

    // Configure APIC timer as periodic
    APIC_Write(APIC_LVT_TIMER, APIC_TIMER_IRQ_VECTOR | APIC_TIMER_PERIODIC);
    APIC_Write(APIC_TIMER_INITCNT, ticksIn10ms); // Set calibrated ticks

    // Register and enable APIC timer IRQ
    APIC_IRQ_RegisterHandler(APIC_TIMER_IRQ_VECTOR, timer_handler);
    APIC_EnableIRQ(APIC_TIMER_IRQ_VECTOR);

    // Unmask APIC timer interrupt
    APIC_Write(APIC_LVT_TIMER, APIC_Read(APIC_LVT_TIMER) & ~APIC_LVT_INT_MASKED);
    APIC_IRQ_RegisterHandler(0, (IRQHandler)timer_handler);
    terminal_printf("APIC Timer Initialized\n");
}

void pit_prepare_sleep(uint32_t microseconds) {
    // Calculate divisor for PIT (avoid rounding errors)
    uint16_t divisor = (uint16_t)((PIT_HZ * microseconds) / 1000000);

    // Configure PIT: Channel 0, Mode 2 (rate generator), Binary
    outb(PIT_COMMAND, 0x34);
    iowait();

    // Send divisor (LSB first, then MSB)
    outb(PIT_CHANNEL0, divisor & 0xFF);
    iowait();
    outb(PIT_CHANNEL0, divisor >> 8);
    iowait();
}

uint16_t pit_read_counter() {
    outb(PIT_COMMAND, 0x00);  // Latch current counter value
    iowait();

    uint16_t count = inb(PIT_CHANNEL0);       // Read low byte
    count |= (inb(PIT_CHANNEL0) << 8);  // Read high byte

    return count;
}

void pit_perform_sleep() {
    uint16_t last_count = pit_read_counter();
    while (1) {
        uint16_t current_count = pit_read_counter();
        if (current_count > last_count) {
            // The counter has wrapped around, meaning the PIT period has elapsed
            break;
        }
        last_count = current_count;
    }
}
