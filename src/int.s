        global default_exception_handler
        global OSASyscall
        global divide_by_zero_handler
        global invalid_opcode_handler
        global timer_interrupt_handler
        global keyboard_interrupt_handler
        global general_protection_fault_handler
        global page_fault_handler
        global test_custom_opcodes
        global LoadAndJump
        extern divide_by_zero
        extern Exception
        extern OSASyscallHandler
        extern invalid_opcode
        extern timer_interrupt
        extern keyboard_handler
        extern general_protection_fault
        extern page_fault
        extern putc

        ; location of temporary registers
        TEMP_REG  equ 0x1000
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
LoadAndJump:
        ; Segments
        mov ax,[TEMP_DS]
        mov bx,[TEMP_SS]
        mov cx,[TEMP_ES]
        mov dx,[TEMP_FS]
        mov si,[TEMP_GS]
        mov ds,ax
        mov ss,bx
        mov es,cx
        mov fs,dx
        mov gs,si

        ; Registers
        mov eax,dword[TEMP_EAX]
        mov ebx,dword[TEMP_EBX]
        mov ecx,dword[TEMP_ECX]
        mov edx,dword[TEMP_EDX]
        mov ebp,dword[TEMP_EBP]
        mov esi,dword[TEMP_ESI]
        mov edi,dword[TEMP_EDI]
        mov esp,dword[TEMP_ESP]

        ; Interrupt Frame
        or dword[TEMP_FLGS],0x200
        push dword[TEMP_FLGS]
        push dword[TEMP_CS]
        push dword[TEMP_EIP]
        iret
invalid_opcode_handler:
        cli
        mov dword[TEMP_EAX],eax
        mov dword[TEMP_EBX],ebx
        mov dword[TEMP_ECX],ecx
        mov dword[TEMP_EDX],edx
        mov dword[TEMP_ESI],esi
        mov dword[TEMP_EDI],edi
        mov dword[TEMP_EBP],ebp
        call invalid_opcode
        mov dword[esp],eax
        mov eax,dword[TEMP_EAX]
        mov ebx,dword[TEMP_EBX]
        mov ecx,dword[TEMP_ECX]
        mov edx,dword[TEMP_EDX]
        mov esi,dword[TEMP_ESI]
        mov edi,dword[TEMP_EDI]
        mov ebp,dword[TEMP_EBP]
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
        ; Registers & CS
        mov dword[TEMP_EAX],eax
        mov dword[TEMP_EBX],ebx
        mov dword[TEMP_ECX],ecx
        mov dword[TEMP_EDX],edx
        mov dword[TEMP_EBP],ebp
        mov dword[TEMP_ESI],esi
        mov dword[TEMP_EDI],edi
        pop dword[TEMP_EIP]
        pop dword[TEMP_CS]
        pop dword[TEMP_FLGS]
        mov dword[TEMP_ESP],esp

        ; Segments
        mov ax,ds
        mov bx,es
        mov cx,ss
        mov dx,fs
        mov si,gs
        mov word[TEMP_DS],ax
        mov word[TEMP_ES],bx
        mov word[TEMP_SS],cx
        mov word[TEMP_FS],dx
        mov word[TEMP_GS],si
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