#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>
#include "io.h"
#include "util.h"

// ---------------------------------------------------------------------------
// Definitions
// ---------------------------------------------------------------------------
#define PAGING_PAGE_SIZE               4096
#define PAGING_NUM_PAGE_DIR_ENTRIES    1024
#define PAGING_NUM_PAGE_TABLE_ENTRIES  1024

// Page table entry flags (x86)
#define PAGING_PAGE_PRESENT    0x001 // - PAGE_PRESENT:     page is present in memory
#define PAGING_PAGE_RW         0x002 // - PAGE_RW:          page is writable
#define PAGING_PAGE_USER       0x004 // - PAGE_USER:        page is accessible from user mode (if needed)
#define PAGING_PAGE_PWT        0x008 // - PAGE_PWT:         page-level write-through
#define PAGING_PAGE_PCD        0x010 // - PAGE_PCD:         page-level cache disable

// For identity mapping (kernel mapping), we may use PRESENT | RW.
#define PAGING_DEFAULT_FLAGS   (PAGING_PAGE_PRESENT | PAGING_PAGE_RW)

// For uncached mappings (e.g. ioremap_nocache), include PWT | PCD.
#define PAGING_UNCACHED_FLAGS  (PAGING_PAGE_PRESENT | PAGING_PAGE_RW | PAGING_PAGE_PWT | PAGING_PAGE_PCD)

// Range to identity map (for example, map the first 16 MB)
#define PAGING_IDENTITY_MAP_SIZE   (16 * 1024 * 1024)


// Enum for page table entry attributes
typedef enum {
    PAGE_PRESENT = 1 << 0,
    PAGE_WRITABLE = 1 << 1,
    PAGE_USER = 1 << 2,
    PAGE_WRITE_THROUGH = 1 << 3,
    PAGE_CACHE_DISABLE = 1 << 4,
    PAGE_ACCESSED = 1 << 5,
    PAGE_DIRTY = 1 << 6,
    PAGE_SIZE = 1 << 7,
    PAGE_GLOBAL = 1 << 8,
    PAGE_AVL = 1 << 9,
    PAGE_FRAME = 0xFFFFF000,
    PAGE_DEFAULT = (PAGE_PRESENT | PAGE_WRITABLE),
    PAGE_UNCACHED = (PAGE_PRESENT | PAGE_WRITABLE | PAGE_WRITE_THROUGH | PAGE_CACHE_DISABLE),
} page_table_entry_flags_t;

// Struct for a page table entry
typedef struct {
    uint32_t present : 1;
    uint32_t writable : 1;
    uint32_t user : 1;
    uint32_t write_through : 1;
    uint32_t cache_disable : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t page_size : 1;
    uint32_t global : 1;
    uint32_t avl : 3;
    uint32_t frame : 20;
} page_table_entry_t;


extern uint32_t *page_directory;

static inline void load_cr3(uint32_t pd_phys_addr) {
    asm("mov %0, %%cr3" : : "r"(pd_phys_addr));
}

static inline void enable_paging() {
    uint32_t cr0;
    asm("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= 0x80000000;  // Set the PG (Paging) bit (bit 31)
    asm("mov %0, %%cr0" : : "r" (cr0));
}

void flush_tlb_range(uint32_t start, uint32_t end);
void set_page_mapping(uint32_t virt_addr, uint32_t phys_addr, page_table_entry_flags_t flags);
void *ioremap_nocache(uint32_t phys_addr, size_t size);
void paging_init();


#endif // PAGING_H