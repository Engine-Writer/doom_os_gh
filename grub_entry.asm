[bits 32]
STACK_SIZE equ 0x4000 ; Define stack size as 16 KB

extern kernel_main
extern _edata
extern _end

global multiboot_data
global _start

; Multiboot2 header setup
section .text
_start:
    jmp full_start

align 8
multiboot2_header_start:
    dd 0xE85250D6               ; Magic number
    dd 0                        ; Reserved
    dd multiboot_end - multiboot2_header_start ; Total header size
    dd -(0xE85250D6 + 0 + (multiboot_end - multiboot2_header_start)) ; Checksum

align 8

INFO_REQ_TAG_START:
    dw 1
    dw 0
    dd INFO_REQ_TAG_END - INFO_REQ_TAG_START
    dd 4
    dd 5
    dd 6
    dd 7
    dd 8
INFO_REQ_TAG_END:

align 8

ADDRESS_TAG_START:
    dw 2 ; type=addr
    dw 0 ; flags=0
    dd ADDRESS_TAG_END - ADDRESS_TAG_START ; size
    dd multiboot2_header_start ; header start addr
    dd _start ; kernel loader
    dd _edata ; code end
    dd _end ; bss end
ADDRESS_TAG_END:

align 8

ENTRY_TAG_START:
    dw 3                     ; Tag type (3 for entry address)
    dw 0                     ; Flags (set to 0 for standard use)
    dd ENTRY_TAG_END - ENTRY_TAG_START ; Tag size (8 bytes)
    dd full_start            ; Entry point address (physical address)
ENTRY_TAG_END:

align 8

; Multiboot2 terminator tag (0)
TERMINATOR_TAG_START:
    dw 0                        ; Tag type (0: terminator)
    dw 0
    dd 8                        ; Tag size (8 bytes)
TERMINATOR_TAG_END:

align 8

multiboot_end:

align 8



full_start:
    cli
    
    mov [multiboot_data], eax
    mov [multiboot_data + 4], ebx
    
    ; Initialize the stack pointer
    lea esp, [stack + STACK_SIZE]

    ; Reset EFLAGS
    push dword 0
    popf

    ; Jump to the kernel's entry point
    jmp kernel_main


section .data
GDT_start:
    ; Null descriptor (always required)
    GDT_null:
        dd 0x0
        dd 0x0

    ; Code segment descriptor (Executable, Readable)
    GDT_code:
        dw 0xffff                 ; Limit: 0xFFFF
        dw 0x0                    ; Base: 0x0000
        db 0x0                    ; Base (lower part)
        db 0b10011010             ; Access: Present, Code Segment, Executable, Readable
        db 0b11001111             ; Granularity: 32-bit, Limit = 4GB, 4K pages
        db 0x0                    ; Base (upper part)

    ; Data segment descriptor (Readable, Writable)
    GDT_data:
        dw 0xffff                 ; Limit: 0xFFFF
        dw 0x0                    ; Base: 0x0000
        db 0x0                    ; Base (lower part)
        db 0b10010010             ; Access: Present, Data Segment, Readable, Writable
        db 0b11001111             ; Granularity: 32-bit, Limit = 4GB, 4K pages
        db 0x0                    ; Base (upper part)
GDT_end:

GDT_descriptor:
    dw GDT_end - GDT_start - 1      ; GDT size
    dd GDT_start                   ; GDT base address

align 16
section .bss
multiboot_data:
    resb 4
    stack resb STACK_SIZE ; Reserve STACK_SIZE (16 KB) for the stack