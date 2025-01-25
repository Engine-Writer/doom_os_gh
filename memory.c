#include "multiboot2.h"
#include "memory.h"
#include "terminal.h"
#include "util.h"

uint32_t heap_start = 0;
uint32_t heap_end = 0;

static block_header_t *free_list = (block_header_t *)0xFFFFFFFF; // Start with no free blocks
uint32_t total_mem = 0;

// Simple memset function (can be expanded if needed)
void *memset(void *ptr, int value, size_t num) {
    unsigned char *p = ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

// Simple memcpy function (can be expanded if needed)
void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

void memory_initialize(uint32_t entry, uint32_t entry_count, multiboot_tag_mmap_t *mmap_tag) {
    for (uint32_t i = 0; i < entry_count; i++) {
        multiboot_mmap_entry_t *entry_x = (multiboot_mmap_entry_t *)( entry+(i*(mmap_tag->entry_size)) );
        // terminal_printf("Entry %d: Base=0x%x, Length=0x%x, Type=%d\n", i, 
        // entry_x->addr_hi, entry_x->len_hi, entry_x->type);

        uint32_t addr_lo = entry_x->addr_lo;
        uint32_t addr_hi = entry_x->addr_hi;
        uint32_t len_lo = entry_x->len_lo;
        uint32_t len_hi = entry_x->len_hi;
        uint32_t type = entry_x->type;

        // If the region is available (Type 1), initialize it
        if (type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint32_t base_addr = addr_hi;
            uint32_t length = len_hi;

            // Check if the region overlaps with the restricted memory (_cstart to _end)
            if ((base_addr+sizeof(block_header_t) < (uint32_t)&_cstart) && (base_addr + length > (uint32_t)&_cstart) ||
            (base_addr+sizeof(block_header_t) < (uint32_t)&_end) && (base_addr + length > (uint32_t)&_end)) {
                if ((base_addr+sizeof(block_header_t) < (uint32_t)&_cstart) && (base_addr + length > (uint32_t)&_cstart)) {
                    length = (uint32_t)&_cstart - base_addr;
                    if (heap_start == 0) {
                        heap_start = base_addr;
                        heap_end = base_addr + length;
                    }

                    // Initialize the free list for available memory regions
                    block_header_t *current_block = (block_header_t *)base_addr;
                    current_block->size = length - sizeof(block_header_t);  // Subtract header size
                    current_block->next = NULL;
                    current_block->is_free = 1;

                    // Add the current block to the free list
                    if (free_list == (void *)0xFFFFFFFF) {
                        free_list = current_block;
                    } else {
                        block_header_t *last_block = free_list;
                        while (last_block->next != NULL) {
                            last_block = last_block->next;
                        }
                        last_block->next = current_block;
                    }
                }
                length += (uint32_t)&_end - (uint32_t)&_cstart;
                if (len_hi > length) {
                    length = len_hi - length;
                    base_addr = (uint32_t)&_end;
                    if (heap_start == 0) {
                        heap_start = base_addr;
                        heap_end = base_addr + length;
                    }

                    // Initialize the free list for available memory regions
                    block_header_t *current_block = (block_header_t *)base_addr;
                    current_block->size = length - sizeof(block_header_t);  // Subtract header size
                    current_block->next = NULL;
                    current_block->is_free = 1;

                    // Add the current block to the free list
                    if (free_list == (void *)0xFFFFFFFF) {
                        free_list = current_block;
                    } else {
                        block_header_t *last_block = free_list;
                        while (last_block->next != NULL) {
                            last_block = last_block->next;
                        }
                        last_block->next = current_block;
                    }
                }
            } else if ((base_addr < _end) && (base_addr + length > _cstart)) {
                // dont do nun
            } else {
                if (heap_start == 0) {
                    heap_start = (uint32_t)base_addr;
                    heap_end = (uint32_t)(base_addr + length);
                }

                // Initialize the free list for available memory regions
                block_header_t *current_block = (block_header_t *)base_addr;
                current_block->size = (uint32_t)length - sizeof(block_header_t);  // Subtract header size
                current_block->next = NULL;
                current_block->is_free = 1;

                // Add the current block to the free list
                if (free_list == (void *)0xFFFFFFFF) {
                    free_list = current_block;
                } else {
                    block_header_t *last_block = free_list;
                    while (last_block->next != NULL) {
                        last_block = last_block->next;
                    }
                    last_block->next = current_block;
                }
            }
        }
    }
}

/* Traverse a Unary Tree, or Node Tree, or something like that idk


block_header_t *last_block = free_list;
while (last_block->next != NULL) {
    last_block = last_block->next;
}

block_header_t *last_block = free_list;
    while (last_block->next != NULL) {
        terminal_printf("Block %x of size %x points at %x and is %d\n", 
        last_block, last_block->size, last_block->next, last_block->is_free);

        last_block = last_block->next;
    }
    terminal_printf("Block %x of size %x points at %x and is %d\n", 
    last_block, last_block->size, last_block->next, last_block->is_free);

*/


// Allocate a block of memory
void *memalloc(size_t size) {
    block_header_t *current = free_list;

    // Align to 8 bytes
    size = (size + 7) & ~7;

    while (current->next != 0x0) {
        // Check if the block is free and large enough
        if (current->is_free && current->size >= size) {
            // If the block is much larger than the requested size, split it
            if (current->size > size + sizeof(block_header_t)) {
                block_header_t *new_block = (block_header_t *)((uint8_t *)current + sizeof(block_header_t) + size);
                new_block->size = current->size - size - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = current->next;

                current->size = size;
                current->next = new_block;
            }

            // Mark the block as allocated
            current->is_free = 0;
            return (void *)((uint32_t)((uint8_t *)current + sizeof(block_header_t))+1);
        }

        current = current->next;
    }

    // No suitable block found, 
    // NULL could very well point to block 0 so
    // I used 0xFFFFFFFF to be safe
    // TODO: REPLACE WITH OUT_OF_RAM ERR
    // WHEN I ADD INTERUPTS
    return (void *)0xFFFFFFFF; 
}



// Free a block of memory
void memfree(void *ptr) {
    if (!ptr) return;

    block_header_t *block = (block_header_t *)((uint8_t *)ptr - sizeof(block_header_t));
    block->is_free = 1;

    // Merge adjacent free blocks
    block_header_t *current = free_list;

    while (current) {
        if (current->is_free && current->next && current->next->is_free) {
            current->size += sizeof(block_header_t) + current->next->size;
            current->next = current->next->next;
        }
        current = current->next;
    }
}

uint32_t get_total_memory() {
    return total_mem;
}