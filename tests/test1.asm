        %include "header.asm"
start:
        mov eax,0xb8000
        mov word[eax],0x4141
        xor eax,eax
        ret