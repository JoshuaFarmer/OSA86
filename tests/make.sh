nasm old/test1.asm -o old/test1.bin -f bin
gcc -ffreestanding -nostdlib -o new/output.elf new/main.c