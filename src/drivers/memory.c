#include "multiboot2.h"
#include "memory.h"
#include "acpi.h"
#include "terminal.h"
#include "util.h"

uint32_t heap_start = 0;
uint32_t heap_end = 0;

static block_header_t *free_list = (block_header_t *)0xFFFFFFFF; // Start with no free blocks
uint32_t total_mem = 0;

void *memset_pattern(void *ptr, const void *pattern, size_t pattern_size, size_t num) {
    unsigned char *p = (unsigned char *)ptr;
    const unsigned char *pat = (const unsigned char *)pattern;
    size_t pat_idx = 0;

    for (size_t i = 0; i < num; ++i) {
        p[i] = pat[pat_idx];
        pat_idx = (pat_idx + 1) % pattern_size;
    }

    return ptr;
}

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

int8_t memcmp(const char *str1, const char *str2, size_t n) {
    while (n-- > 0) {
        if (*str1 != *str2) {
            return (unsigned char)(*str1) - (unsigned char)(*str2);
        }
        str1++;
        str2++;
    }
    return 0;  // Return 0 if strings are equal up to n characters
}

void *memmove(void *dst, const void *src, size_t n) {
    // OK since we know that memcpy copies forwards
    if (dst < src) {
        return memcpy(dst, src, n);
    }

    uint8_t *d = dst;
    const uint8_t *s = src;

    for (size_t i = n; i > 0; i--) {
        d[i - 1] = s[i - 1];
    }

    return dst;
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
    void *some_space = (void *)0xFFFFFFFF;
    uint32_t mem_so_far = 0;
    for (uint32_t i = 0; i < entry_count; i++) {
        multiboot_mmap_entry_t *entry_x = (multiboot_mmap_entry_t *)( entry+(i*(mmap_tag->entry_size)) );
        // terminal_printf("Entry %d: Base=0x%x, Length=0x%x, Type=%d\n", i, 
        // entry_x->addr_hi, entry_x->len_hi, entry_x->type);

        uint32_t addr_lo = entry_x->addr_lo;
        uint32_t addr_hi = entry_x->addr_hi;
        uint32_t len_lo = entry_x->len_lo;
        uint32_t len_hi = entry_x->len_hi;
        uint32_t type = entry_x->type;

        // If the region is acpi reclaimable (Type 3), initialize it
        if (type == MULTIBOOT_MEMORY_ACPI_RECLAIMABLE) {
            memrealloc(some_space, mem_so_far+sizeof(multiboot_mmap_entry_t)+4);
            memcpy((void *)((uint32_t)some_space+mem_so_far), entry_x, sizeof(multiboot_mmap_entry_t));
            acpi_header_t *acpi_addr = (acpi_header_t *)addr_hi; 
            char ntstr[5];
            memcpy(ntstr, acpi_addr->signature, 4);
            terminal_printf("ACPI table found!\nSignature: %s, Length 0x%x, Revision %d\n",
            (char *)&ntstr, acpi_addr->length, acpi_addr->revision);
            if (memcmp((char *)&ntstr, "RSDT", 4) == 0) {
                acpi_init(acpi_addr);
            } else if (memcmp((char *)&ntstr, "XSDT", 4) == 0) {
                acpi_init(acpi_addr);
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

void *memrealloc(void *ptr, size_t new_size) {
    new_size = (new_size + 7) & ~7;
    if (new_size == 0) {
        if (ptr != (void *)0xFFFFFFFF) {
            memfree(ptr);
        }
        return (void *)0xFFFFFFFF; // Return valid NULL equivalent
    }
    if (ptr == (void *)0xFFFFFFFF) {
        return memalloc(new_size);
    }

    block_header_t *block = (block_header_t *)((uint8_t *)ptr - sizeof(block_header_t));

    // Check if the current block is large enough to satisfy the request
    if (block->size >= new_size) {
        return ptr; // No need to move; current block is sufficient
    }

    // Allocate a new block
    void *new_ptr = memalloc(new_size);
    if (new_ptr == (void *)0xFFFFFFFF) {
        return (void *)0xFFFFFFFF; // Allocation failed
    }

    // Copy data from the old block to the new block
    memcpy(new_ptr, ptr, block->size);

    // Free the old block
    memfree(ptr);

    return new_ptr;
}

// Allocate and zero-initialize memory (calloc)
void *memcalloc(size_t num, size_t size) {
    size_t total_size = num * size;

    // Allocate the memory
    void *ptr = memalloc(total_size);
    if (ptr == (void *)0xFFFFFFFF) {
        return (void *)0xFFFFFFFF; // Allocation failed
    }

    // Zero-initialize the memory
    memset(ptr, 0, total_size);
    return ptr;
}

// Allocate a block of memory with a specified alignment
void *mem_alloc_aligned(size_t size, size_t alignment) {
    // Ensure alignment is a power of two
    if ((alignment & (alignment - 1)) != 0 || alignment == 0) {
        return (void *)0xFFFFFFFF; // Invalid alignment
    }

    block_header_t *current = free_list;

    // Round up size to the nearest multiple of alignment
    size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);

    while (current != NULL) {
        // Check if block is free and can accommodate worst-case offset + aligned_size
        if (current->is_free && current->size >= aligned_size + (alignment - 1)) {
            uint8_t *start_ptr = (uint8_t *)current + sizeof(block_header_t);
            uintptr_t start_addr = (uintptr_t)start_ptr;
            uintptr_t offset = (alignment - (start_addr % alignment)) % alignment;
            uint8_t *aligned_ptr = start_ptr + offset;

            size_t total_used = offset + aligned_size;

            // Check if remaining space can be split into a new block
            if (current->size >= total_used + sizeof(block_header_t) + 1) {
                block_header_t *new_block = (block_header_t *)(aligned_ptr + aligned_size);
                new_block->size = current->size - total_used - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = current->next;

                current->size = total_used;
                current->next = new_block;
            }

            current->is_free = 0;
            return aligned_ptr;
        }
        current = current->next;
    }

    // No suitable block found
    return (void *)0xFFFFFFFF;
}


uint32_t get_total_memory() {
    return total_mem;
}