# Save this as build.sh
rm -f *.o *.elf *.bin
nasm -f elf32 boot.asm -o boot.o
gcc -m32 -c kernel.c -o kernel.o -ffreestanding -O2 -Wall
ld -m elf_i386 -n -T linker.ld -o funnyos.elf boot.o kernel.o --build-id=none
objcopy -O binary funnyos.elf funnyos.bin
echo "Build Complete. Hexdump of start:"
hexdump -C funnyos.bin | head -n 1
