#!/bin/bash

# Assemble boot.asm
nasm src/boot.asm -o bin/raw/boot.bin

# Compile kernel.c
cd src
clang -m32 -c -ffreestanding kernel.c -o ../bin/raw/kernel.o -Wall -Wextra
clang -m32 -c -ffreestanding UpsaCPUEmu/main.c -o ../bin/raw/upsa.o -Wall -Wextra
nasm -felf32 "int.s" -o "../bin/raw/int.o"
nasm -felf32 "disk.s" -o "../bin/raw/disk.o"
cd ..

# Link kernel object file
ld -m elf_i386 -T linker.ld bin/raw/kernel.o bin/raw/int.o bin/raw/disk.o bin/raw/upsa.o -o bin/raw/kernel.elf -Os

# Convert ELF to binary
llvm-objcopy -O binary bin/raw/kernel.elf bin/raw/kernel.bin

# Strip kernel.elf
llvm-strip bin/raw/kernel.elf

# Display sections of kernel.elf
llvm-objdump -h bin/raw/kernel.elf

# Combine boot.bin and kernel.bin into an image
cat bin/raw/boot.bin bin/raw/kernel.bin > bin/raw/OSA86.img

# diskutil
clang diskutil.c -o diskutil.o -O3 -Wall

# disks
./diskutil.o "bin/raw/OSA86.img" "bin/OSA86.img"
./diskutil.o "|0|" "bin/BlankDisk.img"

qemu-system-i386 -drive file=bin/OSA86.img,format=raw,if=ide,readonly=off