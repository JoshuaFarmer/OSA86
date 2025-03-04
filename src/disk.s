                        global __ata_lba_read
                        global __ata_lba_write

                        ;=============================================================================
                        ; ATA read sectors (LBA mode) 
                        ;
                        ; @param EAX Logical Block Address of sector
                        ; @param CL  Number of sectors to read
                        ; @param edi The address of buffer to put data obtained from disk
                        ; @param DL The drive number
                        ;
                        ; @return None
                        ;=============================================================================
__ata_lba_read:
                        pushfd
                        and eax, 0x0FFFFFFF
                        push eax
                        push ebx
                        push ecx
                        push edx
                        push edi
                        mov ebx, eax
                        mov edx, 0x01F6
                        shr eax, 24
                        or al, 11100000b
                        out dx, al
                        mov edx, 0x01F2
                        mov al, cl
                        out dx, al
                        mov edx, 0x1F3
                        mov eax, ebx
                        out dx, al
                        mov edx, 0x1F4
                        mov eax, ebx
                        shr eax, 8
                        out dx, al
                        mov edx, 0x1F5
                        mov eax, ebx
                        shr eax, 16
                        out dx, al
                        mov edx, 0x1F7
                        mov al, 0x20
                        out dx, al
.still_going:
                        in al, dx
                        test al, 8
                        jz .still_going
                        mov eax, 256
                        xor bx, bx
                        mov bl, cl
                        mul bx
                        mov ecx, eax
                        mov edx, 0x1F0
                        rep insw
                        pop edi
                        pop edx
                        pop ecx
                        pop ebx
                        pop eax
                        popfd
                        ret

                        ;=============================================================================
                        ; ATA write sectors (LBA mode) 
                        ;
                        ; @param EAX Logical Block Address of sector
                        ; @param CL  Number of sectors to write
                        ; @param edi The address of data to write to the disk
                        ; @param DL The drive number
                        ;
                        ; @return None
                        ;=============================================================================
__ata_lba_write:
                        pushfd
                        and eax, 0x0FFFFFFF
                        push eax
                        push ebx
                        push ecx
                        push edx
                        push edi
                        mov ebx, eax
                        mov edx, 0x01F6
                        shr eax, 24
                        or al, 11100000b
                        out dx, al
                        mov edx, 0x01F2
                        mov al, cl
                        out dx, al
                        mov edx, 0x1F3
                        mov eax, ebx
                        out dx, al
                        mov edx, 0x1F4
                        mov eax, ebx
                        shr eax, 8
                        out dx, al
                        mov edx, 0x1F5
                        mov eax, ebx
                        shr eax, 16
                        out dx, al
                        mov edx, 0x1F7
                        mov al, 0x30
                        out dx, al
.still_going:
                        in al, dx
                        test al, 8
                        jz .still_going
                        mov eax, 256
                        xor bx, bx
                        mov bl, cl
                        mul bx
                        mov ecx, eax
                        mov edx, 0x1F0
                        mov esi, edx
                        rep outsw
                        pop edi
                        pop edx
                        pop ecx
                        pop ebx
                        pop eax
                        popfd
                        ret