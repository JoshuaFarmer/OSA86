        [org 0x7C00]
        [bits 16]

        KERNEL_SIZE  equ 128
        KERNEL_STACK equ 0xFFFF-4
        jmp _start
        nop
	sector_bytes: dw 512
	cluster_sects: db 0
	reserved_bpb: dw 0
	FATs: db 2
	roots: dw 0
	total_sects: dw 5
	media: db 0x80
	FAT_sects: dw 0
	track_sects: dw 18
	head_cnt: dw 2
	hidden_sects: dd 0
	large_sects: dd 0
	sects_per_32: dd 0
	flags: dw 0
	fat_v: dw 0
	root_clust: dd 0
	fsinfo: dw 0
	backup: dw 0
	reserved_e: dd 0 
	dd 0 
	dd 0 
	drive_num: db 0x80
	nt_flags: db 0
	sig: db 0x28
	volume: dd 0
	volume_name: db "OSA86"
	sys: db "FAT32 " ; well thats a fucking lie
_start:
	cli
	xor ax, ax
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov sp, 0x7C00
	push dx
	sti
	mov ah, 2
	mov al, KERNEL_SIZE
	mov cx, 2
	; Some bad BIOS'es don't pass the disk number.
	cmp dl, 0xFF
	je from_floppy
	cmp dl, 0x80
	jge from_disk
	mov dl, 0x80
	jmp from_disk
from_floppy:
	mov bp, 5
	jmp from_disk
check_if_repeat:
	sub bp, 1
	cmp bp, 0
	je spin
from_disk: 
	mov bx, 0x1000
	mov es, bx
	xor bx, bx
	int 0x13
	cmp dl, 0xFF
	je check_for_errors
check_for_errors:
	jnc done_with_disk
	test ah,ah
	jnz spin
done_with_disk:
        mov ah, 0x88
        int 0x15
        jc no_ram
        mov [0x1000],ax
switch32:
	lgdt [gdtinfo]
	cli
	mov eax, cr0
	or  eax, 1
	mov cr0, eax
	jmp clear_prefetch
	nop
	nop
clear_prefetch:
	mov ax, 0x10
	mov ds, ax
	mov ss, ax 
	mov esp,KERNEL_STACK
	mov ax, 0x8
	mov es, ax
	mov fs, ax
	mov gs, ax
        xor eax,eax
        mov ax,[0x1000]
        push eax
	db 0x66
	db 0xEA
	dd Really32
	dw 0x0008
no_ram:
        xor ax,ax
        mov [0x1000],ax
        jmp switch32
        [bits 32]
Really32:
        mov eax, cr0
        test eax, 1
        jz bruh_fail
	call 0x10000
.shutdown:
	jmp $
        [bits 16]
bruh_fail:
        mov ah, 0x0E
        mov al, 'b'
        int 0x10
        mov al, 'r'
        int 0x10
        mov al, 'u'
        int 0x10
        mov al, 'h'
        int 0x10
        jmp $

spin:
	hlt
inf:
	jmp inf

gdtinfo:
	dw gdt_end - gdt - 1 ; Limit (16 bits), subtract 1 because the limit is inclusive
	dd gdt			   ; Base (32 bits)

gdt:
	gdt_null:
		dd 0 ; Null descriptor
		dd 0
	gdt_code:
		dw 0xFFFF ; Limit (16 bits)
		dw 0	  ; Base (16 bits)
		db 0	  ; Base (8 bits)
		db 10011010b ; Access byte
		db 11001111b ; Granularity
		db 0	  ; Base (8 bits)
	gdt_data:
		dw 0xFFFF ; Limit (16 bits)
		dw 0	  ; Base (16 bits)
		db 0	  ; Base (8 bits)
		db 10010010b ; Access byte
		db 11001111b ; Granularity
		db 0	  ; Base (8 bits)
gdt_end:

times 510-($ - $$) db 0
dw 0xAA55
