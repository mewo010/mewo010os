#!/bin/bash
# Clean up
rm -f *.o myos.bin

# 1. Assemble
nasm -f elf32 boot.s -o boot.o

# 2. Compile
gcc -m32 -c kernel.c -o kernel.o -ffreestanding -O2 -Wall -Wextra

# 3. Link (Forces the correct page size and removes warnings)
ld -m elf_i386 -T linker.ld -o myos.bin boot.o kernel.o -z max-page-size=0x1000 --no-warn-rwx-segments

# 4. Verify and Run
echo "--- CHECKING KERNEL ---"
if grub-file --is-x86-multiboot myos.bin; then
    echo "SUCCESS: myos.bin is a valid kernel."
    echo "Starting QEMU..."
    qemu-system-i386 -kernel myos.bin -machine type=pc-i440fx-3.1
else
    echo "ERROR: Multiboot header not found. Check your linker.ld!"
fi