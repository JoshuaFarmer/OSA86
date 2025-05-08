	.text
	.file	"example.c"
	.globl	__Interrupt                     # -- Begin function __Interrupt
	.p2align	4, 0x90
	.type	__Interrupt,@function
__Interrupt:                            # @__Interrupt
# %bb.0:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	movl	12(%ebp), %eax
	movl	8(%ebp), %eax
	movl	8(%ebp), %edx
	movl	12(%ebp), %esi
	#APP
	movl	%edx, %eax
	movl	$0, %ebx
	movl	%esi, %ecx
	int	$128
	retl
	#NO_APP
	xorl	%eax, %eax
	popl	%esi
	popl	%ebp
	retl
.Lfunc_end0:
	.size	__Interrupt, .Lfunc_end0-__Interrupt
                                        # -- End function
	.globl	getch                           # -- Begin function getch
	.p2align	4, 0x90
	.type	getch,@function
getch:                                  # @getch
# %bb.0:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	xorl	%eax, %eax
	movl	$2, (%esp)
	movl	$0, 4(%esp)
	calll	__Interrupt
                                        # kill: def $al killed $al killed $eax
	movsbl	%al, %eax
	addl	$8, %esp
	popl	%ebp
	retl
.Lfunc_end1:
	.size	getch, .Lfunc_end1-getch
                                        # -- End function
	.globl	putc                            # -- Begin function putc
	.p2align	4, 0x90
	.type	putc,@function
putc:                                   # @putc
# %bb.0:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movb	8(%ebp), %al
	movsbl	8(%ebp), %eax
	movl	$1, (%esp)
	movl	%eax, 4(%esp)
	calll	__Interrupt
	addl	$8, %esp
	popl	%ebp
	retl
.Lfunc_end2:
	.size	putc, .Lfunc_end2-putc
                                        # -- End function
	.globl	init                            # -- Begin function init
	.p2align	4, 0x90
	.type	init,@function
init:                                   # @init
# %bb.0:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	calll	main
	movl	%eax, -4(%ebp)
	movl	-4(%ebp), %eax
	xorl	%ecx, %ecx
	movl	$0, (%esp)
	movl	%eax, 4(%esp)
	calll	__Interrupt
.LBB3_1:                                # =>This Inner Loop Header: Depth=1
	jmp	.LBB3_1
.Lfunc_end3:
	.size	init, .Lfunc_end3-init
                                        # -- End function
	.globl	main                            # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
# %bb.0:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%eax
	movl	$65, (%esp)
	calll	putc
	movl	$1, %eax
	addl	$4, %esp
	popl	%ebp
	retl
.Lfunc_end4:
	.size	main, .Lfunc_end4-main
                                        # -- End function
	.ident	"Debian clang version 14.0.6"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym __Interrupt
	.addrsig_sym putc
	.addrsig_sym main
