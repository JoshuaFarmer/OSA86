        global default_exception_handler
        global OSASyscall
        global divide_by_zero_handler
        global invalid_opcode_handler
        global timer_interrupt_handler
        global keyboard_interrupt_handler
        global general_protection_fault_handler
        global page_fault_handler
        extern divide_by_zero
        extern Exception
        extern OSASyscallHandler
        extern invalid_opcode
        extern timer_interrupt
        extern keyboard_handler
        extern general_protection_fault
        extern page_fault
        extern putc
        global LoadAndJump
        global test_custom_opcodes

        ; location of temporary registers
        TEMP_REG  equ 0xFF00
        TEMP_EAX  equ TEMP_REG+(0*4)
        TEMP_EBX  equ TEMP_REG+(1*4)
        TEMP_ECX  equ TEMP_REG+(2*4)
        TEMP_EDX  equ TEMP_REG+(3*4)
        TEMP_ESP  equ TEMP_REG+(4*4)
        TEMP_EBP  equ TEMP_REG+(5*4)
        TEMP_ESI  equ TEMP_REG+(6*4)
        TEMP_EDI  equ TEMP_REG+(7*4)
        TEMP_EIP  equ TEMP_REG+(8*4)
        TEMP_CS   equ TEMP_REG+(9*4)
        TEMP_DS   equ TEMP_REG+(10*4)
        TEMP_ES   equ TEMP_REG+(11*4)
        TEMP_SS   equ TEMP_REG+(12*4)
        TEMP_FS   equ TEMP_REG+(13*4)
        TEMP_GS   equ TEMP_REG+(14*4)
        TEMP_FLGS equ TEMP_REG+(15*4)

        ; stack during swapping program
        TEMP_STCK  equ TEMP_REG-16
        TEMP_STCKS equ 256
LoadAndJump:
        mov esp,TEMP_STCK
        mov eax,[TEMP_FLGS]
        and eax,~0x200
        push eax

        ; segments
        mov ax,[TEMP_DS]
        mov bx,[TEMP_SS]
        mov cx,[TEMP_ES]
        mov dx,[TEMP_FS]
        mov ds,ax
        mov ss,bx
        mov es,cx
        mov fs,dx
        mov ax,[TEMP_GS]
        mov gs,ax

        mov eax,[TEMP_EAX]
        mov ebx,[TEMP_EBX]
        mov ecx,[TEMP_ECX]
        mov edx,[TEMP_EDX]
        mov ebp,[TEMP_EBP]
        mov esi,[TEMP_ESI]
        mov edi,[TEMP_EDI]

        popf
        cli
        mov esp,[TEMP_ESP]
        sti
        jmp far [TEMP_EIP]
invalid_opcode_handler:
        cli
        mov [TEMP_EAX],eax
        mov [TEMP_EBX],ebx
        mov [TEMP_ECX],ecx
        mov [TEMP_EDX],edx
        mov [TEMP_ESI],esi
        mov [TEMP_EDI],edi
        mov [TEMP_EBP],ebp
        call invalid_opcode
        mov [esp],eax
        mov eax,[TEMP_EAX]
        mov ebx,[TEMP_EBX]
        mov ecx,[TEMP_ECX]
        mov edx,[TEMP_EDX]
        mov esi,[TEMP_ESI]
        mov edi,[TEMP_EDI]
        mov ebp,[TEMP_EBP]
        sti
        iret
general_protection_fault_handler:
        cli
        push ds
        push ss
        push es
        push fs
        push gs
        call general_protection_fault
        sti
        iret
page_fault_handler:
        cli
        call page_fault
        sti
        iret
timer_interrupt_handler:
        cli
        mov [TEMP_EAX],eax
        pushf
        pop eax
        mov [TEMP_FLGS],eax
        mov [TEMP_EBX],ebx
        mov [TEMP_ECX],ecx
        mov [TEMP_EDX],edx
        mov ebx,esp
        add ebx,4*3
        mov [TEMP_ESP],ebx
        mov [TEMP_EBP],ebp
        mov [TEMP_ESI],esi
        mov [TEMP_EDI],edi
        pop eax
        pop ebx
        mov [TEMP_EIP],eax
        mov [TEMP_CS],bx
        mov ax,ds
        mov [TEMP_DS],ax
        mov ax,es
        mov [TEMP_ES],ax
        mov ax,ss
        mov [TEMP_SS],ax
        mov ax,fs
        mov [TEMP_FS],ax
        mov ax,gs
        mov [TEMP_GS],ax
        mov ax,0x10
        mov ds,ax
        mov ss,ax
        mov es,ax
        mov fs,ax
        mov gs,ax
        call timer_interrupt
        hlt
        jmp $
keyboard_interrupt_handler:
        cli
        call keyboard_handler
        sti
        iret
default_exception_handler:
	cli
        add esp,4
        call Exception
        sub esp,4
        sti
	iret
divide_by_zero_handler:
	cli
        call divide_by_zero
        sti
	iret
OSASyscall:
	cli
	call OSASyscallHandler
        sti
	iret
test_custom_opcodes:
        mov eax,33
        ud2 ; mov r32,eax
        db 0
        db 32
        db 0
        ud2 ; add eax,r32
        db 3
        db 0
        db 32
        push eax
        call putc
        add esp,4
        mov edi,.msg ; putsn edi
        mov ecx,2
        ud2
        db 21

        int 0x20 ; yield
        ret
.msg:
        db "hi"