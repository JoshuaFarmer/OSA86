#!/bin/bash

nasm src/boot.asm -o bin/raw/boot.bin
cd src
clang -m32 -c -ffreestanding kernel.c -o ../bin/raw/kernel.o -Wall -Wextra -target i386 -Wunused-function -Wno-unused-parameter
nasm -felf32 "int.s" -o "../bin/raw/int.o"
nasm -felf32 "disk.s" -o "../bin/raw/disk.o"
cd ..
ld -m elf_i386 -T linker.ld -o bin/raw/kernel.elf -Os bin/raw/kernel.o bin/raw/int.o bin/raw/disk.o -nostdlib
llvm-objcopy -O binary bin/raw/kernel.elf bin/raw/kernel.bin
llvm-strip bin/raw/kernel.elf
llvm-objdump -h bin/raw/kernel.elf
cat bin/raw/boot.bin bin/raw/kernel.bin > bin/raw/OSA86.img
clang diskutil.c -o diskutil.o -O3 -Wall
./diskutil.o "bin/raw/OSA86.img" "bin/OSA86.img"
./diskutil.o "|0|" "bin/BlankDisk.img"
qemu-system-x86_64 -drive file=bin/OSA86.img,format=raw,if=ide,readonly=off -debugcon stdio