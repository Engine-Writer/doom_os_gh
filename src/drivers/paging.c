// page.c
/*
#include <stdint.h>
#include <stddef.h>
#include "io.h"
#include "util.h"
#include "paging.h"
#include "memory.h"

uint32_t *page_directory = 0xFFFFFFFF;

// ---------------------------------------------------------------------------
// flush_tlb_range: Flush TLB entries for the virtual address range [start, end)
// ---------------------------------------------------------------------------
void flush_tlb_range(uint32_t start, uint32_t end) {
    for (uint32_t addr = start; addr < end; addr += PAGING_PAGE_SIZE) {
        asm("invlpg (%0)" : : "r" (addr) : "memory");
    }
}

// ---------------------------------------------------------------------------
// set_page_mapping: Maps a single page at virtual address 'virt_addr' to physical address 'phys_addr'
// with the given flags. Allocates a new page table if needed.
// ---------------------------------------------------------------------------
void set_page_mapping(uint32_t virt_addr, uint32_t phys_addr, page_table_entry_flags_t flags) {
    // Calculate page directory index (top 10 bits) and page table index (next 10 bits)
    uint32_t pd_index = virt_addr >> 22;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;
    uint32_t *page_table;

    // Check if the page directory entry is present
    if (!(page_directory[pd_index] & PAGING_PAGE_PRESENT)) {
        // Allocate a new page table (must be PAGE_SIZE bytes, page aligned)
        page_table = (uint32_t *)mem_alloc_aligned(PAGING_PAGE_SIZE, PAGING_PAGE_SIZE);
        if (page_table == 0xFFFFFFFF) {
            // Handle allocation failure as needed.
            return;
        }
        memset(page_table, 0, PAGING_PAGE_SIZE);

        // The physical address of the new page table: here we assume identity mapping for simplicity.
        // In a real system, you'd need a function to convert a virtual address from mem_alloc to its physical address.
        uint32_t pt_phys_addr = (uint32_t)page_table;
        // Set the page directory entry: lower 12 bits used for flags.
        page_directory[pd_index] = (pt_phys_addr & 0xFFFFF000) | (PAGING_PAGE_PRESENT | PAGING_PAGE_RW);
    } else {
        // Get the base address of the existing page table.
        page_table = (uint32_t *)(page_directory[pd_index] & 0xFFFFF000);
    }

    // Set the page table entry mapping virt_addr to phys_addr with provided flags.
    page_table[pt_index] = (phys_addr & 0xFFFFF000) | (flags & 0xFFF);
}

// ---------------------------------------------------------------------------
// ioremap_nocache: Map a physical memory region [phys_addr, phys_addr + size)
// into a contiguous virtual address range with uncached attributes.
// Returns the base virtual address.
// ---------------------------------------------------------------------------
void *ioremap_nocache(uint32_t phys_addr, size_t size) {
    // Align size to page boundaries.
    size_t num_pages = (size + PAGING_PAGE_SIZE - 1) / PAGING_PAGE_SIZE;

    // Reserve a contiguous virtual address block.
    void *virt_addr = mem_alloc_aligned(num_pages * PAGING_PAGE_SIZE, PAGING_PAGE_SIZE);
    if (virt_addr == 0xFFFFFFFF)
        return 0xFFFFFFFF;

    uint32_t vaddr = (uint32_t)virt_addr;
    for (size_t i = 0; i < num_pages; i++) {
        uint32_t pa = phys_addr + (i * PAGING_PAGE_SIZE);
        uint32_t va = vaddr + (i * PAGING_PAGE_SIZE);
        set_page_mapping(va, pa, PAGE_UNCACHED);
    }
    // Flush TLB for the new mapping.
    flush_tlb_range(vaddr, vaddr + num_pages * PAGING_PAGE_SIZE);
    return virt_addr;
}

// ---------------------------------------------------------------------------
// paging_init: Set up a basic paging system with an identity mapping for the first part of memory.
// ---------------------------------------------------------------------------
void paging_init() {
    // Allocate a page directory (must be PAGE_SIZE bytes and page aligned).
    page_directory = (uint32_t *)mem_alloc_aligned(PAGING_PAGE_SIZE, PAGING_PAGE_SIZE);
    if (page_directory == 0xFFFFFFFF) {
        // Handle allocation failure.
        return;
    }
    memset(page_directory, 0, PAGING_PAGE_SIZE);

    // Identity map the first IDENTITY_MAP_SIZE bytes.
    for (uint32_t addr = 0; addr < PAGING_IDENTITY_MAP_SIZE; addr += PAGING_PAGE_SIZE) {
        set_page_mapping(addr, addr, PAGE_DEFAULT);
    }

    // Load the page directory (assuming the physical address is the same as the virtual address)
    load_cr3((uint32_t)page_directory);

    // Enable paging in CR0.
    enable_paging();

    // Flush entire TLB (for our identity-mapped region).
    flush_tlb_range(0, PAGING_IDENTITY_MAP_SIZE);
}
*/