[org 0x7C00]
[bits 16]

KERNEL_SIZE equ 128

jmp _start

; We have to start somewhere, we use _start out of convention.
_start:
	push dx
	; We don't want to be disturbed.
	cli
	xor ax, ax
	mov ss, ax
	mov ds, ax
	mov es, ax

	mov sp, 0x7C00
	sti	 

	pusha
	mov ah, 0x3
	int 10h
	popa
	
	; Hide cursor
	mov ah, 0x01
	mov ch, 0x20
	mov cl, 0x1f
	int 0x10

	; Now we're going to load data from disk.
	; No filesystem yet.
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
	; Floppies are unreliable, so we have to try a few times.
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
	cmp ah, 0
	je done_with_disk
	jmp spin

done_with_disk:
	jmp switch32

spin:
	hlt
inf:
	jmp inf

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
	mov esp, 0xD00000
	mov ax, 0x8
	mov es, ax
	mov fs, ax
	mov gs, ax

	db 0x66
	db 0xEA
	dd Really32
	dw 0x0008

[bits 32]

Really32:
	call 0x10000
.shutdown:
	; Switch to real mode
	mov eax, cr0		  ; Read CR0 register
	and eax, 0xFFFFFFFE   ; Clear the PE (Protection Enable) bit
	mov cr0, eax		  ; Write CR0 back

	jmp $

[bits 16]

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
