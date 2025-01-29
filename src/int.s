global default_exception_handler
global OSASyscall
global save_context
global restore_context
global TempEDX

extern Exception
extern puts
extern PRINT_DWORD
extern OSASyscallHandler
extern timer_interrupt_handler
extern schedule_next_task

default_exception_handler:
	cli
        call Exception
	iret
OSASyscall:
	cli
	pusha
	call OSASyscallHandler
	popa
        sti
	iret
