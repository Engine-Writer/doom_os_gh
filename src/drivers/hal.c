#include "multiboot2.h"
#include "terminal.h"
#include "keyboard.h"
#include "memory.h"
#include "timer.h"
#include "hpet.h"
#include "hal.h"
#include "fpu.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "svga.h"
#include "pic_irq.h"
#include "paging.h"
#include "apic_irq.h"
#include "pic.h"
#include "apic.h"
#include "acpi.h"
#include "io.h"
#include <stdint.h>

multiboot_tag_framebuffer_t *fbo_tag_gb;
multiboot_tag_framebuffer_common_t fbo_com_gb;
multiboot_tag_bootdev_t *bootdev_tag;

uint32_t HAL_Initialize(multiboot_info_t *multiboot_info_addr) {
    FPU_Initialize();
    GDT_Initialize();
    IDT_Initialize();
    ISR_Initialize();

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
        terminal_printf("TAG TYPE %d WITH SIZE %x AND ADDRESS %x\n", tag->type, tag->size, tag);
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
                break;
            }
            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
                multiboot_tag_framebuffer_t *fbo_tag = (multiboot_tag_framebuffer_t *)tag;
                multiboot_tag_framebuffer_common_t fbo_com = fbo_tag->common;
                fbo_tag_gb = fbo_tag;
                fbo_com_gb = fbo_com;
/*
                terminal_printf("Framebuffer of type %d and size 0x%x, pitch 0x%x, size (%d x %d) at address 0x%x found!\n",
                    fbo_com.framebuffer_type,
                    fbo_com.size, // If you want to print size, make sure it's correct (either 'size' or calculated manually)
                    fbo_com.framebuffer_pitch,
                    fbo_com.framebuffer_width,
                    fbo_com.framebuffer_height,
                    fbo_com.framebuffer_addr);*/
                
               break;
            }
            case MULTIBOOT_TAG_TYPE_VBE: {
                multiboot_tag_vbe_t *vbe_tag = (multiboot_tag_vbe_t *)tag;

                terminal_printf("VBE::: Mode %d, Mode Info 0x%x, Interface Len 0x%x, Segment 0x%x, found!\n",
                    vbe_tag->vbe_mode,
                    vbe_tag->vbe_mode_info, // If you want to print size, make sure it's correct (either 'size' or calculated manually)
                    vbe_tag->vbe_interface_len,
                    vbe_tag->vbe_interface_off,
                    vbe_tag->vbe_interface_seg);
                break;
            }
            case MULTIBOOT_TAG_TYPE_BOOTDEV: {
                multiboot_tag_bootdev_t *boot_tag = (multiboot_tag_bootdev_t *)tag;
                bootdev_tag = boot_tag;
                
                break;
            }
            default: break;
        }
        tag = (multiboot_tag_t *)((uint8_t *)tag+((tag->size + 7)&~7)); // Adjusted to have (uint8_t *) and altered byte aligment
    }
    ACPI_DISABLE(); // Maybe not yet.....
    // paging_init();
    if (apic_enablable() != 0) {
        PIC_IRQ_Initialize();
        PIC_Disable();    // He didn't even live for a second man poor guy
        outb(0x22, 0x70); // Select IMCR register
        outb(0x23, 0x1);  // Disable PICs (enable APIC mode)
        
        APIC_IRQ_Initialize();

        timer_apic_init();
        keyboard_apic_init();
        // sci_apic_init();

        terminal_writestring("APIC INIT\n");
    } else {
        PIC_IRQ_Initialize();

        PIC_IRQ_RegisterHandler((uint8_t)(sci_int&0x00FF), (IRQHandler)acpi_sci_handler);
        
        timer_pic_init();
        keyboard_pic_init();
        
        terminal_writestring("PIC INIT\n");
    }
    terminal_writestring("ABT TO STI\n");
    STI();
    terminal_writestring("STI-ed\n");
    HPET_Initialize();
    terminal_writestring("Doing Something\n");
    pit_prepare_sleep(20000);
    pit_perform_sleep();
    // paging_init();
    terminal_writestring("Something Done!!\n");
    // RenderFrame0();
    return eflagerrs;
}