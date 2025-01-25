#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include "multiboot2.h"

// Declare external variables
extern uint32_t total_mem;
extern uint32_t heap_start;
extern uint32_t heap_end;

extern uint32_t _cstart;
extern uint32_t _end;

typedef struct {
    uint32_t eax_reg;
    multiboot_info_t* ebx_reg;
} __attribute__((packed)) multiboot_data_t;

extern multiboot_data_t multiboot_data;

// Simple memory region structure for heap management
typedef struct block_header {
    uint32_t size;            // Size of the block, 4 bytes
    uint32_t is_free;         // Wether it is free or used, 2 bytes
    struct block_header *next; // Pointer to the next block in the free list, 4 bytes
} block_header_t;

// Function to initialize memory
void memory_initialize(uint32_t entry, uint32_t entry_count, multiboot_tag_mmap_t *mmap_tag);

// Memory allocation functions
void *memalloc(size_t size);
void memfree(void *ptr);

// Utility functions
void *memset(void *ptr, int value, size_t num);
void *memcpy(void *dest, const void *src, size_t n);

// Function to calculate total system memory
uint32_t get_total_memory();

#endif
