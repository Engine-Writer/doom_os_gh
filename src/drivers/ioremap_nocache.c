/*#include <stdint.h>
#include <stddef.h>
#include "memory.h"

// Assume PAGE_SIZE is 4KB.
#define PAGE_SIZE 4096

// Page table entry flag definitions for x86:
// Bit 0: Present (1)
// Bit 1: Read/Write (1 = writable)
// Bit 3: Page-level Write-Through (PWT)
// Bit 4: Page-level Cache Disable (PCD)
#define PAGE_PRESENT   0x001
#define PAGE_RW        0x002
#define PAGE_PWT       0x008
#define PAGE_PCD       0x010

extern uint32_t *page_directory;

/*
 * flush_tlb_range:
 *   Flush (invalidate) the TLB for the virtual address range [start, end).
 *   On x86, we can use the "invlpg" instruction to invalidate one page at a time.
 * /
static void flush_tlb_range(uint32_t start, uint32_t end) {
    for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
        asm volatile ("invlpg (%0)" : : "r" (addr) : "memory");
    }
}

/*
 * set_page_mapping:
 *   Maps a single page at virtual address `virt_addr` to physical address `phys_addr`
 *   with the specified flags.
 *
 *   This function calculates the page directory and page table indices for virt_addr.
 *   If the required page table does not exist, it is allocated using mem_alloc.
 *   Then the page table entry is set to map to phys_addr with the given flags.
 * /
static void set_page_mapping(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    // Calculate the index into the page directory (top 10 bits) and page table (next 10 bits)
    uint32_t pd_index = virt_addr >> 22;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;
    uint32_t *page_table;

    // Check if the page directory entry is present.
    if (!(page_directory[pd_index] & PAGE_PRESENT)) {
        // Allocate a new page table (must be page-aligned).
        page_table = (uint32_t *)mem_alloc(PAGE_SIZE);
        if (!page_table) {
            // Allocation failure; in a real system, you would handle this error.
            return;
        }
        memset(page_table, 0, PAGE_SIZE);
        // Set the page directory entry:
        // Use the physical address of the page table (assumed to be the same as its virtual address here)
        // and mark it as present and writable.
        page_directory[pd_index] = ((uint32_t)page_table & 0xFFFFF000) | (PAGE_PRESENT | PAGE_RW);
    } else {
        // Get the base address of the existing page table.
        page_table = (uint32_t *)(page_directory[pd_index] & 0xFFFFF000);
    }

    // Set the page table entry:
    // Map the virtual page to the physical page with the provided flags.
    page_table[pt_index] = (phys_addr & 0xFFFFF000) | (flags & 0xFFF);
}

/*
 * ioremap_nocache:
 *   Maps a physical address region [phys_addr, phys_addr + size) into a contiguous
 *   virtual address range with uncached attributes.
 *
 *   This implementation:
 *     - Aligns the size to page boundaries.
 *     - Uses mem_alloc to reserve the virtual address space.
 *     - Iterates over each page and calls set_page_mapping() with flags that mark
 *       the page as present, writable, uncached (PCD), and write-through (PWT).
 *     - Flushes the TLB for the new mappings.
 *
 *   Returns the virtual address for the mapping, or NULL on failure.
 * /
void *ioremap_nocache(uint32_t phys_addr, size_t size) {
    // Calculate the number of pages required.
    size_t num_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    // Allocate a contiguous virtual address block.
    void *virt_addr = mem_alloc(num_pages * PAGE_SIZE);
    if (!virt_addr)
        return NULL;

    uint32_t vaddr = (uint32_t)virt_addr;
    for (size_t i = 0; i < num_pages; i++) {
        uint32_t pa = phys_addr + i * PAGE_SIZE;
        uint32_t va = vaddr + i * PAGE_SIZE;
        // Set flags: present, RW, write-through, cache disabled.
        uint32_t flags = PAGE_PRESENT | PAGE_RW | PAGE_PWT | PAGE_PCD;
        set_page_mapping(va, pa, flags);
    }
    // Invalidate the TLB entries for the new virtual range.
    flush_tlb_range(vaddr, vaddr + num_pages * PAGE_SIZE);
    return virt_addr;
}
*/