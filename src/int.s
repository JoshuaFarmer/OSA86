global default_exception_handler
global OSASyscall
global save_context
global restore_context
global timer_isr

extern puts
extern PRINT_DWORD
extern OSASyscallHandler
extern timer_interrupt_handler
extern schedule_next_task

default_exception_handler:
	cli
	iret
OSASyscall:
	cli
	pusha
	call OSASyscallHandler
	popa
	iret

;timer_isr:
;	cli					; Disable interrupts
;	pusha				  ; Save all general-purpose registers
;	call timer_interrupt_handler ; Call your timer handler (task scheduler)
;	mov al, 0x20
;	out 0x20, al		; Send End of Interrupt (EOI) signal to PIC
;	popa				   ; Restore all registers
;	iret				   ; Return from interrupt
;
;; Save context (registers)
;save_context:
;	pusha				   ; Push all general purpose registers
;	push ds				 ; Save segment registers
;	push es
;	push fs
;	push gs
;	mov ax, 0x10			; Load data segment (user data segment selector)
;	mov ds, ax
;	mov es, ax
;	mov fs, ax
;	mov gs, ax
;	jmp schedule_next_task  ; Jump to scheduler to switch tasks
;
;; Restore context (registers)
;restore_context:
;	pop gs				  ; Restore segment registers
;	pop fs
;	pop es
;	pop ds
;	popa					; Pop all general purpose registers
;	iret					; Return from interrupt and continue execution
;