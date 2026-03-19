	.file	"main.c"
	.intel_syntax noprefix
	.text
	.globl	_main
	.def	_main;	.scl	2;	.type	32;	.endef
_main:
	push	ebp
	mov	ebp, esp
	and	esp, -16
	sub	esp, 16
	call	___main
	mov	DWORD PTR [esp+12], 753664
	mov	eax, DWORD PTR [esp+12]
	add	eax, 4
	mov	BYTE PTR [eax], 67
	mov	eax, DWORD PTR [esp+12]
	add	eax, 5
	mov	BYTE PTR [eax], 15
L2:
	jmp	L2
	.globl	___main
	.def	___main;	.scl	2;	.type	32;	.endef
___main:
	push	ebp
	mov	ebp, esp
	nop
	pop	ebp
	ret
	.def	___main;	.scl	2;	.type	32;	.endef
	.ident	"GCC: (GNU) 15.2.0"
