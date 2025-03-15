#include "hpet.h"
#include "paging.h"
#include "timer.h"
#include "memory.h"
#include "io.h"       // for ioremap_nocache(), inb(), outb(), inl(), outl()
#include "terminal.h" // for terminal_printf()
#include "acpi.h"
#include <stdint.h>


// Global variables (assumed to be set by ACPI parsing code)
uint32_t hpet_base_address = 0;
volatile void *hpet_virt_addr = NULL;
uint64_t hpet_io_port = 0;

// Initialize HPET: map registers and enable HPET
void HPET_Initialize() {
    // Get base address from ACPI table.
    hpet_base_address = (uint32_t)hpet_data->base_address.Address;
    if (!hpet_base_address) {
        hpet_base_address = HPET_BASE_ADDRESS;
    }
    uint8_t space = hpet_data->base_address.AddressSpace;
    terminal_printf("HPET MemorySpace: %d\n", space);
    
    if (space == 0) {
        // Memory-mapped: map physical HPET registers.
        hpet_virt_addr = (volatile void *)hpet_base_address;
        if (!hpet_virt_addr) {
            terminal_printf("Failed to map HPET registers!\n");
            return;
        }
        terminal_printf("HPET registers mapped to virtual address: 0x%x\n", (uint32_t)hpet_virt_addr);
    } else if (space == 1) {
        // I/O-mapped: store base port.
        hpet_io_port = hpet_base_address;
        terminal_printf("HPET registers accessed via I/O port: 0x%x\n", (uint32_t)hpet_io_port);
    } else {
        hpet_virt_addr = (volatile void *)hpet_base_address;
        if (!hpet_virt_addr) {
            terminal_printf("Failed to map HPET registers!\n");
            return;
        }
    }
    terminal_printf("HPET Address: 0x%x\n", hpet_base_address);

    // Enable HPET by setting the enable bit in the General Configuration Register.
    uint32_t cfg = HPET_ReadIO(HPET_GENERAL_CONFIGURATION);
    cfg |= HPET_ENABLE_BIT;
    HPET_WriteIO(HPET_GENERAL_CONFIGURATION, cfg);
    terminal_printf("HPET initialized and enabled.\n");
    uint64_t c1, c2;

    // Configure Timer 0 for 10MHz operation (no interrupts)
    HPET_ConfigureTimer(0, HPET_TPS); // 5MHz (adjust freq as needed)
    // pit_prepare_sleep(50000); // 20,000 ms (micro) or about 1 second
    c1 = HPET_ReadCounter();
    terminal_printf("Them timer at 0x%x \n", c1);
    for (int _=0;_<20;++_) {
        pit_prepare_sleep(50000); // 50,000 ms (micro) or about 0.05 second
        pit_perform_sleep();
    }
    c2 = HPET_ReadCounter();
    terminal_printf("Now them timer at 0x%x \n", c2);
    terminal_printf("Elapsed:: 0x%x \n", c2-c1);
}

// HPET_ConfigureTimer: Configure a specific HPET timer for a period given in nanoseconds.
// This function disables HPET temporarily, sets up the timer comparator for the desired period,
// and configures Timer 0 without enabling interrupts.
void HPET_ConfigureTimer(uint32_t timer_id, uint64_t freq_per) {
    // Optionally disable HPET during configuration.
    HPET_WriteIO(HPET_GENERAL_CONFIGURATION, 0);

    // Read HPET capabilities to get the period in femtoseconds per tick.
    uint64_t capabilities = HPET_ReadCapabilities();
    uint32_t hpet_period_fs = (uint32_t)(capabilities >> 32);
    if (hpet_period_fs == 0) {
        terminal_printf("Error: HPET clock period is 0\n");
        return;
    }

    // Calculate HPET frequency in Hz: frequency = 1e15 / hpet_period_fs.
    double hpet_frequency_hz = 1000000000000000 / (double)hpet_period_fs;
    // Desired frequency is 10 MHz.
    double desired_frequency_hz = freq_per;
    // Calculate comparator value (number of ticks between events).
    // If the hardware periodic mode doubles the delay, divide the computed value by 2.
    uint64_t comparator_value = ((uint64_t)(hpet_frequency_hz / desired_frequency_hz)) / 2;

    // Set the comparator value for the specified timer.
    uint32_t comparator_offset = HPET_TIMER_COMPARATOR + (timer_id * 0x20);
    HPET_WriteIO(comparator_offset, (uint32_t)(comparator_value & 0xFFFFFFFF));
    HPET_WriteIO(comparator_offset + 4, (uint32_t)((comparator_value >> 32) & 0xFFFFFFFF));

    // Configure the timer: disable interrupts and enable the timer.
    // (Assuming the interrupt enable bit is bit 2 and timer enable bit is bit 0.)
    uint32_t config_offset = HPET_TIMER_CONFIG + (timer_id * 0x20);
    uint32_t config = HPET_ReadIO(config_offset);
    config &= ~(1 << 2); // Disable interrupts.
    config |= 1;         // Enable the timer.
    HPET_WriteIO(config_offset, config);

    // Re-enable HPET by setting the enable bit in the General Configuration register.
    uint32_t general_cfg = HPET_ReadIO(HPET_GENERAL_CONFIGURATION);
    general_cfg |= HPET_ENABLE_BIT;
    HPET_WriteIO(HPET_GENERAL_CONFIGURATION, general_cfg);
    
    terminal_printf("HPET Timer %d configured: Comparator = %u ticks (Desired frequency: %f Hz, HPET frequency: %f Hz)\n",
     timer_id, comparator_value, desired_frequency_hz, hpet_frequency_hz);
}

void HPET_Sleep(float seconds) {
    // Calculate the number of ticks to wait
    uint64_t ticks = (uint64_t)(seconds * HPET_FREQ);

    // Get the current counter value
    uint64_t start_time = HPET_ReadCounter();

    // Wait until the desired time has passed
    while ((HPET_ReadCounter() - start_time) < ticks) {
        // Busy-wait loop
    }
}

void HPET_SleepNS(uint32_t ns) {
    // Calculate the number of ticks to wait
    uint64_t ticks = (uint64_t)(ns * (HPET_FREQ / 1000000000.0));

    // Get the current counter value
    uint64_t start_time = HPET_ReadCounter();

    // Wait until the desired time has passed
    while ((HPET_ReadCounter() - start_time) < ticks) {
        // Busy-wait loop
    }
}

// Example function: Increment a value at each interval of 'interval_ticks'.
// This is a busy-wait loop demonstration.
void HPET_IncrementValueAtInterval(uint32_t interval_ticks) {
    uint64_t last_counter = HPET_ReadCounter();
    uint64_t target_ticks = last_counter + interval_ticks;
    uint64_t value = 0;

    while (1) {
        uint64_t current_counter = HPET_ReadCounter();
        if (current_counter >= target_ticks) {
            value++;
            target_ticks += interval_ticks;
            // For demonstration, you might print or log 'value' periodically.
        }
    }
}
