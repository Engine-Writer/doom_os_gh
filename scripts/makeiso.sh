mkdir -p iso/boot/grub
cp ./bin/doom_os.elf ./iso/boot/doom_os.elf
cp grub.cfg iso/boot/grub/

grub-mkrescue -o ./bin/doom_os.iso ./iso