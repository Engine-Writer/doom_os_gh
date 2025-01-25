OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

SECTIONS
{
    /* Place the Multiboot2 header at an early address within the first 32 KB */
    . = 0x002000;               /* Offset for Multiboot2 header */

    .text ALIGN(16) : {
        KEEP(*(.multiboot2))
        *(.text)
    }

    .data ALIGN(16) : {
        *(.data)                /* Initialized data */
    }
    _edata = .;

    .bss ALIGN(16) : {
        *(COMMON)               /* Common symbols */
        *(.bss)                 /* Uninitialized data */
    }
    _end = .;
}
