#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "multiboot2.h"
#include "terminal.h"
#include "memory.h"
#include "timer.h"
#include "sound.h"
#include "util.h"
#include "gdt.h"
#include "hal.h"
#include "idt.h"
#include "isr.h"
#include "pic_irq.h"
#include "apic_irq.h"
#include "pic.h"
#include "apic.h"
#include "io.h"


// Kernel entry point (called by the bootloader)
void kernel_main() {
    char dumparea[64];
    if (multiboot_data.eax_reg != 0x36D76289) {
        terminal_initialize();
        terminal_writestring("Invalid Magic Code: 0x");
        terminal_writestring(itoa(multiboot_data.eax_reg, dumparea, 16));
        terminal_writestring("! Please Install GRUB2 as this OS is GRUB2 Dependent.");
        while (true) {
            // Infinite Loop...
        }
    }
    
    terminal_initialize();
    terminal_clear(make_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLUE));
    uint32_t err_flags = HAL_Initialize(multiboot_data.ebx_reg);

    if (err_flags&1) {
        terminal_writestring("Error: reserved_data_not_zero. \nPlease Reboot, Properly Install RAM, or Reinstall GRUB2.");
        while (true)
        {
            // Infinite Loop...
        }
        
    }

    terminal_writestring("Welcome to DOOM OS! \n");
    terminal_set_cursor_position(make_uint8_vector2(0, 0));
    
    uint32_t ticks = 0;
    
    // Infinite loop to keep the kernel running
    while (true) {
        sound_play_frequency(220);
        ticks = timer_ticks;
        while (timer_ticks-ticks > 10) {
            iowait();
        }
    }
}
