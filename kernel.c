#include <stdint.h>
#include "multiboot2.h"
#include "terminal.h"
#include "memory.h"
#include "util.h"
#include "gdt.h"
#include "io.h"


/*
void loop_tags(multiboot_info_t *addr) {
    multiboot_tag_t *tag;
    unsigned size;

    // Check if addr is aligned to 8 bytes
    if ((uint32_t)addr & 7) {
        terminal_printf("Unaligned mbi: 0x%x\n", addr);
        return;
    }

    // Get the total size of the Multiboot information
    size = *(unsigned *) addr;
    terminal_printf("Announced mbi size 0x%x\n", size);

    // Iterate through the tags
    for (tag = (multiboot_tag_t *)(addr + 8); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (multiboot_tag_t *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7))) {
        terminal_printf("Tag 0x%x, Size 0x%x\n", tag->type, tag->size);

        // Handle different types of tags
        switch (tag->type) {
        case MULTIBOOT_TAG_TYPE_CMDLINE:
            terminal_printf("Command line = %s\n", ((multiboot_tag_string_t *)tag)->string);
            break;
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
            terminal_printf("Boot loader name = %s\n", ((multiboot_tag_string_t *)tag)->string);
            break;
        case MULTIBOOT_TAG_TYPE_MODULE:
            terminal_printf("Module at 0x%x-0x%x. Command line %s\n",
                            ((multiboot_tag_module_t *)tag)->mod_start,
                            ((multiboot_tag_module_t *)tag)->mod_end,
                            ((multiboot_tag_module_t *)tag)->cmdline);
            break;
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            terminal_printf("mem_lower = %uKB, mem_upper = %uKB\n",
                            ((multiboot_tag_basic_meminfo_t *)tag)->mem_lower,
                            ((multiboot_tag_basic_meminfo_t *)tag)->mem_upper);
            break;
        case MULTIBOOT_TAG_TYPE_BOOTDEV:
            terminal_printf("Boot device 0x%x,%u,%u\n",
                            ((multiboot_tag_bootdev_t *)tag)->biosdev,
                            ((multiboot_tag_bootdev_t *)tag)->slice,
                            ((multiboot_tag_bootdev_t *)tag)->part);
            break;
        case MULTIBOOT_TAG_TYPE_MMAP:
            {
                multiboot_memory_map_t *mmap;
                terminal_printf("mmap\n");

                for (mmap = ((multiboot_tag_mmap_t *)tag)->entries;
                     (multiboot_uint8_t *)mmap < (multiboot_uint8_t *)tag + tag->size;
                     mmap = (multiboot_memory_map_t *)((unsigned long)mmap + ((multiboot_tag_mmap_t *)tag)->entry_size)) {
                    terminal_printf(" base_addr = 0x%x%x,"
                                    " length = 0x%x%x, type = 0x%x\n",
                                    (unsigned)(mmap->addr >> 32),
                                    (unsigned)(mmap->addr & 0xffffffff),
                                    (unsigned)(mmap->len >> 32),
                                    (unsigned)(mmap->len & 0xffffffff),
                                    (unsigned)mmap->type);
                }
            }
            break;
        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
            {
                multiboot_uint32_t color;
                unsigned i;
                multiboot_tag_framebuffer_t *tagfb = (multiboot_tag_framebuffer_t *)tag;
                void *fb = (void *)(unsigned long)tagfb->common.framebuffer_addr;

                switch (tagfb->common.framebuffer_type) {
                case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
                    {
                        unsigned best_distance, distance;
                        multiboot_color_t *palette;

                        palette = tagfb->framebuffer_palette;
                        color = 0;
                        best_distance = 4 * 256 * 256;

                        for (i = 0; i < tagfb->framebuffer_palette_num_colors; i++) {
                            distance = (0xff - palette[i].blue) * (0xff - palette[i].blue)
                                       + palette[i].red * palette[i].red
                                       + palette[i].green * palette[i].green;
                            if (distance < best_distance) {
                                color = i;
                                best_distance = distance;
                            }
                        }
                    }
                    break;
                case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
                    color = ((1 << tagfb->framebuffer_blue_mask_size) - 1)
                            << tagfb->framebuffer_blue_field_position;
                    break;
                case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
                    color = '\\' | 0x0100;
                    break;
                default:
                    color = 0xffffffff;
                    break;
                }

                for (i = 0; i < tagfb->common.framebuffer_width && i < tagfb->common.framebuffer_height; i++) {
                    switch (tagfb->common.framebuffer_bpp) {
                    case 8:
                        {
                            multiboot_uint8_t *pixel = fb + tagfb->common.framebuffer_pitch * i + i;
                            *pixel = color;
                        }
                        break;
                    case 15:
                    case 16:
                        {
                            multiboot_uint16_t *pixel = fb + tagfb->common.framebuffer_pitch * i + 2 * i;
                            *pixel = color;
                        }
                        break;
                    case 24:
                        {
                            multiboot_uint32_t *pixel = fb + tagfb->common.framebuffer_pitch * i + 3 * i;
                            *pixel = (color & 0xffffff) | (*pixel & 0xff000000);
                        }
                        break;
                    case 32:
                        {
                            multiboot_uint32_t *pixel = fb + tagfb->common.framebuffer_pitch * i + 4 * i;
                            *pixel = color;
                        }
                        break;
                    }
                }
            }
            break;
        default:
            terminal_printf("Unhandled tag type: 0x%x\n", tag->type);
            break;
        }
    }

    // Calculate and print the total size of the MBI
    tag = (multiboot_tag_t *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7));
    terminal_printf("Total mbi size 0x%x\n", (unsigned)tag - (uint32_t)addr);
}
*/


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

    terminal_printf("EBX Register: 0x%x\n", multiboot_data.ebx_reg);
    terminal_printf("Total Size: 0x%x\n", size_total);
    terminal_printf("Reserved: 0x%x\n", reserved_data);

    // If reserved data is not zero, handle the error
    if (reserved_data != 0) {
        terminal_writestring("Error: reserved_data_not_zero");
    }
    
    uint32_t mb2_end = multiboot_data.ebx_reg->total_size + (uint32_t)multiboot_data.ebx_reg;

    multiboot_tag_t *tag = (multiboot_tag_t*)((uint8_t *)multiboot_data.ebx_reg + 8); // Adjusted to have (uint8_t *)
    while ((uint32_t)tag < mb2_end)
    {
        terminal_printf("TAG TYPE %d WITH SIZE %x AND ADDRESS %x\n", tag->type, tag->size, tag);
        if (tag->type == 0)
            break;
                switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_MMAP: {
                // Memory map tag found, parse it
                multiboot_tag_mmap_t *mmap_tag = (multiboot_tag_mmap_t *)tag;

                // Parse each memory region in the memory map
                uint32_t entry = (uint32_t)mmap_tag+16;
                uint32_t entry_count = mmap_tag->size / mmap_tag->entry_size;
                terminal_printf("Found memory map: %x of size %d\n", mmap_tag, mmap_tag->size);

                for (uint32_t i = 0; i < entry_count; i++) {
                    multiboot_mmap_entry_t *entry_x = (multiboot_mmap_entry_t *)( entry+(i*(mmap_tag->entry_size)) );
                    terminal_printf("Entry %d: Base=0x%x, Length=0x%x, Type=%d\n", i, 
                    entry_x->addr_hi, entry_x->len_hi, entry_x->type);
                }
                break;
            }
            default: break;
        }

        tag = (multiboot_tag_t *) ((uint8_t *) tag + ((tag->size + 7) & ~7)); // Adjusted to have (uint8_t *) and altered byte aligment
    }

    // Infinite loop to keep the kernel running
    while (1) {
        // Kernel running
    }
}
