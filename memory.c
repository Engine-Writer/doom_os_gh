#include "memory.h"
#include "terminal.h"
#include "util.h"

// Define the start and end addresses of the heap (using the memory map)
#define HEAP_START 0x00101000   // Start at 2MB for heap
#define HEAP_END   0x00102000   // 256 B of ram cuz ummm idk

static block_header_t *free_list = NULL; // Start with no free blocks

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
/*
// Initialize memory by iterating through the memory map and setting up the heap
void memory_initialize() {
    memory_map_entry_t *mmap = (memory_map_entry_t *)multiboot_info->mmap_addr;
    uint32_t mmap_length = multiboot_info->mmap_length;
    char memdump[32];  // Array to store the string for debugging
    char *memdump_ptr = memdump;  // Pointer to the buffer

    for (uint32_t i = 0; i < mmap_length; i += mmap[i].size + sizeof(mmap[i])) {
        // Print index i in hexadecimal (or decimal) for debugging purposes
        memdump_ptr = itoa(mmap[i].base_addr, memdump_ptr, 10);  // Convert index to string (assuming 'itoa' works correctly)
        memdump[31] = '\0';     // Ensure null-termination
        terminal_writestring(memdump);  // Print the current index

        if (mmap[i].type == 1) {  // Usable RAM
            terminal_writestring(" Usable");  // Print "Usable"

            uint64_t start = mmap[i].base_addr;
            uint64_t end = mmap[i].base_addr + mmap[i].length;

            if (start >= HEAP_START && end <= HEAP_END) {
                // Align the start of the heap to an 8-byte boundary and initialize free list
                free_list = (block_header_t *)((start + 7) & ~0x7);
                free_list->size = (size_t)(end - start);
                free_list->next = NULL;
            }
        }
        // Print a new line after processing a memory region
        terminal_writestring("\n");
    }
}
*/

// Allocate a block of memory
void *memalloc(size_t size) {
    block_header_t *current = free_list;
    block_header_t *prev = NULL;

    // Align to 8 bytes for better memory alignment
    size = (size + 7) & ~7;

    while (current) {
        if (current->size >= size) {
            // Found a block large enough for allocation
            if (current->size > size + sizeof(block_header_t)) {
                // Split the block if it's large enough to leave some memory for future allocations
                block_header_t *new_block = (block_header_t *)((uint8_t *)current + sizeof(block_header_t) + size);
                new_block->size = current->size - size - sizeof(block_header_t);
                new_block->next = current->next;
                current->size = size;
                current->next = new_block;
            }

            // Remove the block from the free list
            if (prev) {
                prev->next = current->next;
            } else {
                free_list = current->next;
            }

            // Return the memory address after the block header
            return (void *)((uint8_t *)current + sizeof(block_header_t));
        }

        prev = current;
        current = current->next;
    }

    // No suitable block found
    return NULL;
}

// Free a block of memory
void memfree(void *ptr) {
    if (!ptr) return;

    block_header_t *block = (block_header_t *)((uint8_t *)ptr - sizeof(block_header_t));

    // Add the block back to the free list
    block->next = free_list;
    free_list = block;
}

/* Function to calculate total system memory
uint32_t get_total_memory() {
    uint32_t total_memory = 0;
    memory_map_entry_t *mmap = (memory_map_entry_t *)multiboot_info->mmap_addr;

    // Loop through the memory map and sum the lengths of all memory regions
    for (uint32_t i = 0; i < multiboot_info->mmap_length; i += mmap[i].size + sizeof(mmap[i].size)) {
        if (mmap[i].type == 1) {  // Type 1 indicates usable RAM
            total_memory += mmap[i].length;
        }
    }

    // Convert to MB
    return total_memory / 1024;  // Directly return total memory in KB
}
*/