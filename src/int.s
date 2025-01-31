global default_exception_handler
global OSASyscall
global jump_usermode
global divide_by_zero_handler
global invalid_opcode_handler
global timer_interrupt_handler
global keyboard_interrupt_handler
global general_protection_fault_handler
global page_fault_handler
extern divide_by_zero
extern Exception
extern OSASyscallHandler
extern test_user_function
extern invalid_opcode
extern timer_interrupt
extern keyboard_handler
extern general_protection_fault
extern page_fault
extern putc
extern put_int
global LoadAndJump
extern PRINT_DWORD
extern PRINT_DWORD_NE
extern clearScreen

LoadAndJump:
        mov esp,[esp+4]
        pop eax
        pop ebx
        pop ecx
        pop edx
        pop esi
        pop edi
        pop ebp
        popf
        pop ds
        pop es
        pop fs
        pop gs
        sti
        retf
        cli
        hlt
        jmp $
invalid_opcode_handler:
        cli
        call invalid_opcode
        jmp $
        sti
        iret
general_protection_fault_handler:
        cli
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
        push ebp
        mov ebp,esp
        push eax
        mov eax,dword[ebp+8]
        mov dword[0xFFFF0C],eax
        mov eax,dword[ebp+4]
        mov dword[0xFFFF10],eax

        pop eax
        pop ebp
        add esp,8
        mov dword[0xFFFF00],esp
        mov esp,0xFFF000
        push dword[0xFFFF0C]
        push dword[0xFFFF10]
        push gs
        push fs
        push es
        push ss
        push ds
        pushf
        push dword[0xFFFF00]
        push ebp
        push edi
        push esi
        push edx
        push ecx
        push ebx
        push eax
        mov ax, 0x10
        mov ds, ax
        mov ss, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        call timer_interrupt
        sti
        mov esp,[0xFFFF00]
        iret
keyboard_interrupt_handler:
        cli
        call keyboard_handler
        sti
        iret
        iret
        iret
        iret
        iret
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
