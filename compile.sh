#!/bin/bash
export PATH=$PATH:/usr/local/i386elfgcc/bin

nasm -f elf32 grub_entry.asm -o ./bin/kernel_entry.o
nasm -f elf32 gdt.asm -o ./bin/gdt_asm.o

i386-elf-gcc -ffreestanding -m32 -g -c kernel.c -o ./bin/kernel_main.o
i386-elf-gcc -ffreestanding -m32 -g -c terminal.c -o ./bin/terminal.o
i386-elf-gcc -ffreestanding -m32 -g -c memory.c -o ./bin/memory.o
i386-elf-gcc -ffreestanding -m32 -g -c util.c -o ./bin/util.o
i386-elf-gcc -ffreestanding -m32 -g -c gdt.c -o ./bin/gdt_c.o

i386-elf-ld -T linker.ld -o ./bin/doom_os.elf ./bin/kernel_entry.o   \
 ./bin/kernel_main.o ./bin/memory.o ./bin/util.o ./bin/terminal.o    \
 ./bin/gdt_asm.o ./bin/gdt_c.o

sh ./makeiso.sh
sh ./run.sh