OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

SECTIONS
{
    /* Place the Multiboot2 header at an early address within the first 32 KB */
    . = 0x007000;               /* Offset for Multiboot2 header */

    _cstart = .;
    .text ALIGN(16) : {
        *(.text)
    }

    .rodata ALIGN(16) : {
        *(.rodata)
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
