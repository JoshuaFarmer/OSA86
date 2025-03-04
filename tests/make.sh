nasm old/test1.asm -o old/test1.bin -f bin
gcc new/main.c -o new/main -m32
gcc new/main.c -o new/main.s -S -m32
cd new
../fstools/bin/bin2c main main.o.c