#!/bin/bash

# Configuration
QEMU_PATH="/mnt/c/Program Files/qemu/qemu-system-i386.exe"

echo "--- FunnyOS GUI Build Started ---"

# 1. Clean old files to ensure a fresh update
rm -rf *.o *.elf *.iso isodir

# 2. Compile and Link
nasm -f elf32 boot.asm -o boot.o || exit
gcc -m32 -c kernel.c -o kernel.o -ffreestanding -O2 -Wall || exit
ld -m elf_i386 -n -T linker.ld -o funnyos.elf boot.o kernel.o --build-id=none || exit

# 3. Create ISO Structure
mkdir -p isodir/boot/grub
cp funnyos.elf isodir/boot/funnyos.elf

# 4. Generate GRUB Config (Set to 0 timeout for speed)
cat << EOF > isodir/boot/grub/grub.cfg
set timeout=0
set default=0
menuentry "FunnyOS GUI" {
    multiboot /boot/funnyos.elf
    boot
}
EOF

# 5. Final ISO Build
grub-mkrescue -o funnyos.iso isodir || exit

echo "--- Build Successful! Launching... ---"

# In run.sh, change step 6 to ONLY this:
qemu-system-i386 -cdrom funnyos.iso -m 1G -vga std
