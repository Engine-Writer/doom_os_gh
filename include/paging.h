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



// Enum for Page Properties
/*
typedef enum {
    PAGE_PRESENT         = 0x001, // Page is present in memory
    PAGE_RW              = 0x002, // Read/Write permission
    PAGE_USER            = 0x004, // User/Supervisor permission
    PAGE_PWT             = 0x008, // Page-level Write-Through
    PAGE_PCD             = 0x010, // Page-level Cache Disable
    PAGE_ACCESSED        = 0x020, // Page has been accessed
    PAGE_DIRTY           = 0x040, // Page has been written to
    PAGE_4MB             = 0x080, // 4 MB page (only in page directory)
    PAGE_GLOBAL          = 0x100, // Global page
    PAGE_PAT             = 0x200, // Page Attribute Table
    PAGE_PROTECTED_KEY   = 0x400, // Protection key
    PAGE_EXECUTE_DISABLE = 0x800  // Execute Disable
} PageProperty;
*/
// Enum for page table entry attributes
typedef enum {
    PAGE_PRESENT         = 0x001,
    PAGE_WRITABLE        = 0x002,
    PAGE_USER            = 0x004,
    PAGE_PWT             = 0x008,
    PAGE_PCD             = 0x010,
    PAGE_ACCESSED        = 0x020,
    PAGE_DIRTY           = 0x040,
    PAGE_4MB             = 0x080,
    PAGE_GLOBAL          = 0x100,
    PAGE_PAT             = 0x200,
    PAGE_PROTECTED_KEY   = 0x400, // Protection key
    PAGE_EXECUTE_DISABLE = 0x800,  // Execute Disable
    PAGE_FRAME           = 0xFFFFF000,
    PAGE_DATA            = (PAGE_PRESENT | PAGE_WRITABLE | PAGE_EXECUTE_DISABLE | PAGE_USER),
    PAGE_SYSDATA         = (PAGE_PRESENT | PAGE_WRITABLE | PAGE_EXECUTE_DISABLE),
    PAGE_RODATA          = (PAGE_PRESENT | PAGE_EXECUTE_DISABLE | PAGE_USER),
    PAGE_SYSRODATA       = (PAGE_PRESENT | PAGE_EXECUTE_DISABLE),
    PAGE_DEFAULT         = (PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER),
    PAGE_SYSDEFAULT      = (PAGE_PRESENT | PAGE_WRITABLE),
    PAGE_UNCACHED        = (PAGE_PRESENT | PAGE_WRITABLE | PAGE_PWT | PAGE_PCD),
} PageProperty;

extern uint32_t _cstart;
extern uint32_t _ecode;
extern uint32_t _rdstart;
extern uint32_t _erodata;
extern uint32_t _dstart;
extern uint32_t _edata;
extern uint32_t _bssstart;
extern uint32_t _ebss;
extern uint32_t _end;
// Struct for a page table entry
/*typedef struct {
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
} page_table_entry_t;*/


// extern uint32_t page_directory;

static inline void load_cr3(uint32_t pd_phys_addr) {
    asm("mov %0, %%cr3" : : "r"(pd_phys_addr));
}

static inline void enable_paging(uint32_t *page_directory) {
    asm("mov %0, %%cr3" : : "r" (page_directory));

    // Enable paging by setting PG and PE bits in CR0
    uint32_t cr0;
    asm("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= 0x80000001;
    asm("mov %0, %%cr0" : : "r" (cr0));
}

void flush_tlb_range(uint32_t start, uint32_t end);
void paging_init();
void set_page_mapping(uint32_t virt_addr, uint32_t phys_addr, PageProperty flags);
void unmap_page(void* virtualaddr);


#endif // PAGING_H