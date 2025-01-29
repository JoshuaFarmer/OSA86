global default_exception_handler
global OSASyscall
global jump_usermode
global divide_by_zero_handler
global invalid_opcode_handler
extern divide_by_zero
extern Exception
extern OSASyscallHandler
extern test_user_function
extern invalid_opcode

invalid_opcode_handler:
        cli
        call invalid_opcode
        iret
default_exception_handler:
	cli
        add esp,4
        call Exception
        sub esp,4
	iret
divide_by_zero_handler:
	cli
        call divide_by_zero
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