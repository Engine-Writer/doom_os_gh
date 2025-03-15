#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "multiboot2.h"
#include "keyboard.h"
#include "terminal.h"
#include "memory.h"
#include "timer.h"
#include "sound.h"
#include "atapi.h"
#include "util.h"
#include "math.h"
#include "hpet.h"
#include "svga.h"
#include "disk.h"
#include "gdt.h"
#include "hal.h"
#include "idt.h"
#include "isr.h"
#include "pic_irq.h"
#include "apic_irq.h"
#include "pic.h"
#include "apic.h"
#include "io.h"

void Meh_Another_Bug_Init(Registers *regs) {
    printf("A close 1\n");
    HALT();
}
void Meh_Another_Bug2_Init(Registers *regs) {
    printf("A close 2\n");
    HALT();
}

// Kernel entry point (called by the bootloader)
void kernel_main() {
    char dumparea[64];
    terminal_initialize();
    if (multiboot_data.eax_reg != 0x36D76289) {
        printf("Invalid Magic Code: 0x%x! Please Install GRUB2 as this OS is GRUB2 Dependent.", multiboot_data.eax_reg);
        while (true);
    }
    
    uint32_t err_flags = HAL_Initialize(multiboot_data.ebx_reg);

    if (err_flags&1) {
        printf("Error: reserved_data_not_zero. \nPlease Reboot, Properly Install RAM, or Reinstall GRUB2.");
        while (true);
    }

    multiboot_data.dl_reg = (uint32_t)(bootdev_tag->biosdev);
    //ISR_RegisterHandler(0, Meh_Another_Bug_Init);
    //ISR_RegisterHandler(13, Meh_Another_Bug2_Init);


    terminal_clear(make_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLUE));
    
    printf("Welcome to DOOM OS!\n");
    printf("Boot Device: 0x%x\n", (uint32_t)(bootdev_tag->biosdev));
    printf("Boot Partition: %u\n", (uint32_t)(bootdev_tag->part));
    printf("Boot Sub Partition: %u\n", (uint32_t)(bootdev_tag->slice));
    
    //int8_t atapi_ret = ATAPI_Initialize();
    bool atapi_ret;
    printf("ABT TO DISK");
    DEBUG_LABEL_SYMBOL:

    DISK disk;
    if (!DISK_Initialize(&disk, (uint8_t)(bootdev_tag->biosdev&0xFF)))
    {
        printf("Disk init error\n");
        HALT();
    }

    printf("ABT TO READIO");

    if (atapi_ret){
        atapi_ret = DISK_ReadSectors(&disk, 16, 1, (void *)read_buffer);
    }
    
    if (atapi_ret != 0){
        printf("Error: reserved_data_not_zero. \nPlease Reboot, Properly Install RAM, or Reinstall GRUB2.\n");
    } else {
        printf("Error: Sigma.\n");
    }

    HPET_Sleep(.5f);
    
    printf("ISO Signature: %x-%x-%x-%x-%x\n\n", 
        read_buffer[1], read_buffer[2], read_buffer[3], read_buffer[4], read_buffer[5]);
    printf("Memory: %u MB (%u MiB)\n", (uint32_t)(get_total_memory()/(1000*1000)), (uint32_t)(get_total_memory()/(1024*1024)));
    volatile char *command_memory = (volatile char *)memalloc(512);
    printf("Command Address: 0x%x\n\n", command_memory);
    uint16_t letter_index = 0;
    
    uint32_t ticks = 0;
    
    // Infinite loop to keep the kernel running
    uint64_t hpet_tick_old = HPET_ReadCounter();
    uint64_t hpet_tick_current = 0;
    uint64_t tick_hpet_start_tick = HPET_ReadCounter();
    printf("root@recoverymedia:/media/cdrom/$ ");
    while (true) {
        hpet_tick_current = HPET_ReadCounter(); // How many ticks rn
        // RenderStuff( (uint32_t)(hpet_tick_current - hpet_tick_old) ); // send
        // printf("One Second Has Passed!! 0x%x \n", hpet_tick_current - hpet_tick_old);
        hpet_tick_old = hpet_tick_current; // now the current tick count became old
        HPET_Sleep(0.0001);                // Anti NUKE

        for (char char_index=32; char_index<127; ++char_index) {
            if (keyboard.chars[char_index]) {
                printf("%c", char_index);
                command_memory[letter_index] = char_index;
                command_memory[letter_index+1] = 0;
                ++letter_index;

                keyboard.chars[char_index] = false;
            }
        }
        if (keyboard.chars['\n']) {
            printf("\n");
            printf("Executing Command: %s\n", command_memory);

            keyboard.chars['\n'] = false;

            letter_index = 0;
            terminal_writestring("root@recoverymedia: /media/cdrom/$ ");
        }
        if (keyboard.chars['\b'] && letter_index==0) {
            keyboard.chars['\b'] = false;
        } else if (keyboard.chars['\b']) {
            keyboard.chars['\b'] = false;
            command_memory[letter_index-1] = 0;
            if (terminal_column > 0) {
                --terminal_column;
            } else {
                --terminal_row;
                terminal_column = VGA_WIDTH-1;
            }
            printf(" ");            
            if (terminal_column > 0) {
                --terminal_column;
            } else {
                --terminal_row;
                terminal_column = VGA_WIDTH-1;
            }

            --letter_index;
        }
        
        terminal_set_cursor_position(make_uint8_vector2(terminal_column, terminal_row));

        //pit_prepare_sleep((uint32_t)roundf((float)(1000000/24)));
        //pit_perform_sleep();
    }
}
