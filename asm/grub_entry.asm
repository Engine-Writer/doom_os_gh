[bits 32]
STACK_SIZE equ 0x4000 ; Define stack size as 16 KB

[extern kernel_main]
[extern _edata]
[extern _end]

[global multiboot_data]

[global _start]
[global HALT]

[global read_buffer]

; Multiboot2 header setup
section .multiboot
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
    dd 16
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

;FBO_REQ_TAG_START:
;    dw 5           ; Tag type (5 for framebuffer)
;    dw 0           ; Flags
;    dd 20          ; Size of the tag
;    dd 640         ; Preferred width (e.g., 1024 pixels)
;    dd 480         ; Preferred height (e.g., 768 pixels)
;    dd 32          ; Preferred depth (e.g., 32 bits per pixel)
;FBO_REQ_TAG_END:

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


section .text
full_start:
    cli
    
    mov [multiboot_data], eax
    mov [multiboot_data + 4], ebx

    
    ; Initialize the stack pointer
    lea esp, [stack + STACK_SIZE]
    ; Reset EFLAGS
    push dword 0
    popf

continue_:

    ; Jump to the kernel's entry point
    call kernel_main
    jmp HALT

READCD_ISODATA:
    ret
    


HALT:
    cli
    hlt
    jmp HALT


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
    dd GDT_start                    ; GDT base address

disk_packet:
    db 0x10        ; Packet size (must be 16 bytes)
    db 0           ; Reserved (always 0)
    dw 1           ; Read 1 sector
    dw 6420 ; Offset of buffer (low 16 bits)
    dw 0           ; Segment of buffer
    dq 16          ; LBA sector 16 (Primary Volume Descriptor)


align 16
section .bss
multiboot_data:
    resb 20
stack: 
    resb STACK_SIZE ; Reserve STACK_SIZE (16 KB) for the stack

old_gdt:
    resb 6

old_idt:
    resb 6

read_buffer:
    resb 512