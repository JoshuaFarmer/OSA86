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
extern temp1
extern temp2
extern temp3
extern putc
global LoadAndJump
LoadAndJump:
        popf
        pop eax
        pop eax
        pop eax
        pop eax
        pop edi
        pop esi
        pop ebp
        mov [0xFF00],ebp
        pop ebp
        pop ebx
        pop edx
        pop ecx
        pop eax
        mov [0xF04],ecx
        mov [0xF08],eax
        pop ecx
        pop eax
        mov esp,[0xF00]
        push eax
        push ecx
        mov ecx,[0xF04]
        mov eax,[0xF08]
        retf
invalid_opcode_handler:
        cli
        call invalid_opcode
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
        push gs
        push fs
        push es
        push ss
        push ds
        pushf
        push esp
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
        add esp,4*14
        iret
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
	pusha
	call OSASyscallHandler
	popa
        sti
	iret
jump_usermode:
        mov ebp,esp
        mov ax,0x10|3
        mov ds,ax
        mov es,ax
        mov fs,ax
        mov gs,ax
        xor edx,edx
        mov eax,0x8
        mov ecx,0x174
        wrmsr
        mov edx,[ebp+4]
        mov ecx,esp
        sysexit