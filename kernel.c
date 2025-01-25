#include <stdint.h>
#include "multiboot2.h"
#include "terminal.h"
#include "memory.h"
#include "util.h"
#include "gdt.h"
#include "io.h"


// Kernel entry point (called by the bootloader)
void kernel_main() {
    char dumparea[64];
    terminal_initialize();
    i686_GDT_Initialize();

    // Check the magic number
    if (multiboot_data.eax_reg != 0x36D76289) {
        terminal_writestring("Invalid Magic Code: 0x");
        terminal_writestring(itoa(multiboot_data.eax_reg, dumparea, 16));
        terminal_writestring("! Please Install GRUB2 as this OS is GRUB2 Dependent.");
        while (1) {
            // Infinite Loop...
        }
    }

    terminal_clear(make_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLUE));
    terminal_writestring("Welcome to DOOM OS! \n");
    terminal_set_cursor_position(make_uint8_vector2(0, 0));

    // Debugging multiboot data
    uint32_t size_total = multiboot_data.ebx_reg->total_size;
    uint32_t reserved_data = multiboot_data.ebx_reg->reserved;

    // If reserved data is not zero, handle the error
    if (reserved_data != 0) {
        terminal_writestring("Error: reserved_data_not_zero. \nPlease Reboot, Properly Install RAM, or Reinstall GRUB2.");
        while (1)
        {
            // Infinite Loop....
        }
        
    }
    
    uint32_t mb2_end = multiboot_data.ebx_reg->total_size + (uint32_t)multiboot_data.ebx_reg;
    
    uint32_t mmap_tag_addr = 0;
    uint32_t mmap_entry_count = 0;
    uint32_t mmap_first_entry_addr = 0;

    multiboot_tag_t *tag = (multiboot_tag_t*)((uint8_t *)multiboot_data.ebx_reg + 8); // Adjusted to have (uint8_t *)
    while ((uint32_t)tag < mb2_end) {
        // terminal_printf("TAG TYPE %d WITH SIZE %x AND ADDRESS %x\n", tag->type, tag->size, tag);
        if (tag->type == 0)
            break;
                switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_MMAP: {
                // Memory map tag found, parse it
                multiboot_tag_mmap_t *mmap_tag = (multiboot_tag_mmap_t *)tag;

                // Parse each memory region in the memory map
                uint32_t entry_count = mmap_tag->size / mmap_tag->entry_size;
                uint32_t entry = (uint32_t)mmap_tag+16;

                memory_initialize(entry, entry_count, mmap_tag);
                break;
            }
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
                multiboot_tag_basic_meminfo_t *meminfo_tag = (multiboot_tag_basic_meminfo_t *)tag;
                total_mem = (meminfo_tag->mem_lower + meminfo_tag->mem_upper) * 1024;
            }
            default: break;
        }
        tag = (multiboot_tag_t *)((uint8_t *)tag+((tag->size + 7)&~7)); // Adjusted to have (uint8_t *) and altered byte aligment
    }
    
    // Infinite loop to keep the kernel running
    while (1) {
        // Kernel running
    }
}
