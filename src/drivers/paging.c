// page.c
#include <stdint.h>
#include <stddef.h>
#include "terminal.h"
#include "paging.h"
#include "memory.h"
#include "util.h"
#include "idt.h"
#include "isr.h"
#include "io.h"

#define PAGE_SIZE 4096
#define NUM_ENTRIES 1024

uint32_t page_directory[NUM_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
uint32_t* page_tables[NUM_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

// ---------------------------------------------------------------------------
// flush_tlb_range: Flush TLB entries for the virtual address range [start, end)
// ---------------------------------------------------------------------------
void flush_tlb_range(uint32_t start, uint32_t end) {
    for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
        asm("invlpg (%0)" : : "r" (addr) : "memory");
    }
}

void page_fault_handler(Registers *regs) {
    uint32_t fault_addr;
    // Retrieve the faulting address from CR2
    asm("mov %%cr2, %0" : "=r" (fault_addr));

    // Print out basic page fault information
    terminal_printf("Page fault at address: 0x%x\n", fault_addr);
    terminal_printf("Error code: 0x%x\n", regs->error);

    // Interpret the error code:
    // Bit 0: Present flag (0 = not present, 1 = protection fault)
    // Bit 1: Write flag (0 = read, 1 = write)
    // Bit 2: User flag (0 = kernel mode, 1 = user mode)
    // Bit 3: Reserved bit violation
    // Bit 4: Instruction fetch (if applicable)
    if (!(regs->error & 0x1)) {
        terminal_printf("Cause: Page not present.\n");
    } else {
        terminal_printf("Cause: Protection violation.\n");
    }
    if (regs->error & 0x2) {
        terminal_printf("Operation: Write.\n");
    } else {
        terminal_printf("Operation: Read.\n");
    }
    if (regs->error & 0x4) {
        terminal_printf("Fault occurred in user mode.\n");
    } else {
        terminal_printf("Fault occurred in kernel mode.\n");
    }

    // Here you might add handling logic to load a page from disk, allocate a new page, etc.
    // For now, we'll simply halt the system.
    terminal_printf("Halting system due to page fault.\n");
}

// ---------------------------------------------------------------------------
// paging_init: Set up a basic paging system with an identity mapping for the first part of memory.
// ---------------------------------------------------------------------------
void paging_init() {
    terminal_printf("Initializing paging...\n");
    ISR_RegisterHandler(14, (ISRHandler)page_fault_handler);

    // Zero out the page directory
    for (size_t i = 0; i < NUM_ENTRIES; i++) {
        page_directory[i] = 0;
    }

    // Create a page table for the first 4 MiB
    uint32_t* first_page_table = (uint32_t*)mem_alloc_aligned(PAGE_SIZE, PAGE_SIZE);
    if (first_page_table == NULL) {
        terminal_printf("Error: Failed to allocate memory for the first page table.\n");
        return;
    }
    memset(first_page_table, 0, PAGE_SIZE);

    // Map the first 4 MiB of physical memory to the first 4 MiB of virtual memory
    for (size_t i = 0; i < NUM_ENTRIES; i++) {
        first_page_table[i] = (i * PAGE_SIZE) | PAGE_SYSDEFAULT;
    }

    // Set the first entry of the page directory to point to the first page table
    page_directory[0] = ((uint32_t)first_page_table) | PAGE_DEFAULT;

    // Load the page directory address into CR3
    enable_paging(page_directory);

    terminal_printf("Paging enabled successfully.\n");
}

// ---------------------------------------------------------------------------
// set_page_mapping: Maps a single page at virtual address 'virt_addr' to physical address 'phys_addr'
// with the given flags. Allocates a new page table if needed.
// ---------------------------------------------------------------------------
void set_page_mapping(uint32_t virt_addr, uint32_t phys_addr, PageProperty flags) {
    // Calculate page directory index (top 10 bits) and page table index (next 10 bits)
    uint32_t pd_index = virt_addr >> 22;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;
    uint32_t* page_table;

    // Check if the page directory entry is present
    if (!(page_directory[pd_index] & PAGING_PAGE_PRESENT)) {
        // Allocate a new page table (must be PAGE_SIZE bytes, page aligned)
        page_table = (uint32_t*)mem_alloc_aligned(PAGE_SIZE, PAGE_SIZE);
        if (page_table == NULL) {
            // Handle allocation failure as needed.
            return;
        }
        memset(page_table, 0, PAGE_SIZE);

        // The physical address of the new page table: here we assume identity mapping for simplicity.
        // In a real system, you'd need a function to convert a virtual address from mem_alloc to its physical address.
        uint32_t pt_phys_addr = (uint32_t)page_table;
        // Set the page directory entry: lower 12 bits used for flags.
        page_directory[pd_index] = (pt_phys_addr & 0xFFFFF000) | (PAGING_PAGE_PRESENT | PAGING_PAGE_RW);
    } else {
        // Get the base address of the existing page table.
        page_table = (uint32_t*)(page_directory[pd_index] & 0xFFFFF000);
    }
    // Set the page table entry mapping virt_addr to phys_addr with provided flags.
    page_table[pt_index] = (phys_addr & 0xFFFFF000) | (flags & 0xFFF);
}

// ---------------------------------------------------------------------------
// unmap_page: Unmaps a single page at the given virtual address.
// ---------------------------------------------------------------------------
void unmap_page(void* virtualaddr) {
    uint32_t pd_index = (uint32_t)virtualaddr >> 22;
    uint32_t pt_index = ((uint32_t)virtualaddr >> 12) & 0x3FF;
    uint32_t* page_table = (uint32_t*)(page_directory[pd_index] & 0xFFFFF000);

    // Mark the page as not present
    page_table[pt_index] = 0;

    // Invalidate the TLB for the unmapped page
    asm("invlpg (%0)" : : "r" (virtualaddr) : "memory");
}




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