#include "multiboot2.h"
#include "memory.h"
#include "timer.h"
#include "hal.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "pic.h"
#include <stdint.h>


uint32_t HAL_Initialize(multiboot_info_t *multiboot_info_addr) {
    GDT_Initialize();
    IDT_Initialize();
    ISR_Initialize();
    IRQ_Initialize();
    
    timer_set_pit_frequency(125);
    IRQ_RegisterHandler(0, (IRQHandler)timer);

    STI();

    // Debugging multiboot data
    uint32_t size_total = multiboot_info_addr->total_size;
    uint32_t reserved_data = multiboot_info_addr->reserved;

    // If reserved data is not zero, handle the error
    if (reserved_data != 0) {
        return 1;
    }

    
    uint32_t mb2_end = multiboot_info_addr->total_size + (uint32_t)multiboot_info_addr;
    
    uint32_t mmap_tag_addr = 0;
    uint32_t mmap_entry_count = 0;
    uint32_t mmap_first_entry_addr = 0;
    uint32_t eflagerrs = 6;

    multiboot_tag_t *tag = (multiboot_tag_t*)((uint8_t *)multiboot_info_addr + 8); // Adjusted to have (uint8_t *)
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
                eflagerrs -= 2;
                break;
            }
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
                multiboot_tag_basic_meminfo_t *meminfo_tag = (multiboot_tag_basic_meminfo_t *)tag;
                total_mem = (meminfo_tag->mem_lower + meminfo_tag->mem_upper) * 1024;
                eflagerrs -= 4;
            }
            default: break;
        }
        tag = (multiboot_tag_t *)((uint8_t *)tag+((tag->size + 7)&~7)); // Adjusted to have (uint8_t *) and altered byte aligment
    }
    return eflagerrs;
}