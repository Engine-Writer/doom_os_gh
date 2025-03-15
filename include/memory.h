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
    uint32_t dl_reg;
} __attribute__((packed)) multiboot_data_t;

extern multiboot_data_t multiboot_data;

// Simple memory region structure for heap management
typedef struct block_header {
    uint32_t size;            // Size of the block, 4 bytes
    uint32_t is_free;         // Wether it is free or used, 4 bytes
    uint32_t reserved;        // Align 8
    struct block_header *next; // Pointer to the next block in the free list, 4 bytes
} __attribute__((packed)) block_header_t;

// Function to initialize memory
void memory_initialize(uint32_t entry, uint32_t entry_count, multiboot_tag_mmap_t *mmap_tag);

// Memory allocation functions
void *memalloc(size_t size);
void memfree(void *ptr);

// Utility functions
void *memset(void *ptr, int value, size_t num);
void *memcpy(void *dest, const void *src, size_t n);
int8_t memcmp(const char *str1, const char *str2, size_t n);
void *memmove(void *dst, const void *src, size_t n);

void *memrealloc(void *ptr, size_t new_size);
void *memcalloc(size_t num, size_t size);
void *mem_alloc_aligned(size_t size, size_t alignment);

// Function to calculate total system memory
uint32_t get_total_memory();


void *memset_pattern(void *ptr, const void *pattern, size_t pattern_size, size_t num);

#endif // MEMORY_H