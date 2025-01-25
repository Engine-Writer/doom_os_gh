#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include "multiboot2.h"

typedef struct {
    uint32_t eax_reg;
    multiboot_info_t* ebx_reg;
} __attribute__((packed)) multiboot_data_t;

extern multiboot_data_t multiboot_data;
// Function to initialize memory
// void memory_initialize();

// Memory allocation functions
// void *memalloc(size_t size);
// void memfree(void *ptr);
void *memset(void *ptr, int value, size_t num);
void *memcpy(void *dest, const void *src, size_t n);

// Function to calculate total system memory
// uint32_t get_total_memory();

// Simple memory region structure for heap management
typedef struct block_header {
    size_t size;            // Size of the block
    struct block_header *next; // Pointer to the next block in the free list
} block_header_t;

#endif
