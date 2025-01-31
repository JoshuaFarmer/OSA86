        %include "header.asm"
start:
        mov ebx,0
        mov eax,1
        push eax
        push ebx
        int 0x80
        jmp $ ; wait