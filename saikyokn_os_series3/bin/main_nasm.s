_main:
	push	ebp
	mov	ebp, esp
	and	esp, -16
	sub	esp, 16
	call	
	mov	DWORD [esp+12], 753664
	mov	eax, DWORD [esp+12]
	add	eax, 4
	mov	BYTE [eax], 67
	mov	eax, DWORD [esp+12]
	add	eax, 5
	mov	BYTE [eax], 15
L2:
	jmp	L2
:
	push	ebp
	mov	ebp, esp
	nop
	pop	ebp
	ret
