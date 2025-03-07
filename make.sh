#!/bin/bash

nasm src/boot.asm -o bin/raw/boot.bin -f bin
nasm src/int.s -o bin/raw/int.o -f elf32
nasm src/disk.s -o bin/raw/disk.o -f elf32
clang src/kernel.c -o bin/raw/kernel.o -m32 -ffast-math -c -ffreestanding -Wall -Wextra -target i386

ld -m elf_i386 -T linker.ld -o bin/raw/kernel.elf -Os bin/raw/kernel.o bin/raw/int.o bin/raw/disk.o -nostdlib
llvm-strip bin/raw/kernel.elf
llvm-objcopy -O binary bin/raw/kernel.elf bin/raw/kernel.bin
llvm-objdump -h bin/raw/kernel.elf

clang -c "osa86 stdlib/example.c" -o "osa86 stdlib/example.o" -m32 -ffast-math -ffreestanding -Wall -Wextra -target i386 -Wunused-function -Wno-unused-parameter
clang -c -S "osa86 stdlib/example.c" -o "osa86 stdlib/example.s" -m32 -ffast-math -ffreestanding -Wall -Wextra -target i386 -Wunused-function -Wno-unused-parameter
ld -T "osa86 stdlib/linker.ld" "osa86 stdlib/example.o" -m elf_i386 -nostdlib -o "osa86 stdlib/example.elf"
llvm-objcopy -O binary "osa86 stdlib/example.elf" "osa86 stdlib/example.bin"

cat bin/raw/boot.bin bin/raw/kernel.bin > bin/raw/OSA86.img

clang fstools/src/diskutil.c -o fstools/bin/diskutil -O3 -Wall
clang fstools/src/osafs2.c -o fstools/bin/osafs -O3 -Wall
./fstools/bin/diskutil "bin/raw/OSA86.img" "bin/OSA86.img"

qemu-system-x86_64 -m 8 -hda bin/OSA86.img -debugcon stdio