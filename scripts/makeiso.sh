mkdir -p iso/boot/grub
cp /home/freedomuser/shared_folder/bin/doom_os.elf /home/freedomuser/shared_folder/iso/boot/doom_os.elf
cp grub.cfg iso/boot/grub/

grub-mkrescue -o /home/freedomuser/shared_folder/bin/doom_os.iso ./iso