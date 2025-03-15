OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

SECTIONS
{
    /* Place the Multiboot2 header at the start of the image */
    . = 0x00000000;               /* Offset for Multiboot2 header */
    .text ALIGN(4096) : {
        _cstart = .;
        *(.multiboot)
    }

    .text ALIGN(4096) : {
        *(.text)
    }
    _ecode = .;

    .rodata ALIGN(4096) : {
        _rdstart = .;
        *(.rodata)
    }
    _erodata = .;

    .data ALIGN(4096) : {
        _dstart = .;
        *(.data)                /* Initialized data */
    }
    _edata = .;

    .bss ALIGN(16) : {
        _bssstart = .;
        *(COMMON)               /* Common symbols */
        *(.bss)                 /* Uninitialized data */
    }
    _ebss = .;

    _end = .;
}
