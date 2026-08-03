.include "m16def.inc"
.cseg
__init:
	ldi r16, high(RAMEND)
	out SPH, r16
	ldi r16, low(RAMEND)
	out SPL, r16
	ldi r16, 0xFF
	out DDRB, r16
	call main
	out PORTB, r24
__exit:
	rjmp __exit
main:
	push r28
	push r29
	in r29, SPH
	in r28, SPL
	sbiw r28, 0
	out SPH, r29
	out SPL, r28
	; prologue end, stack size = 0
	ldi r16, 3
	ldi r17, 1
	add r16, r17
	ldi r17, 2
	mov r16, r17
	sub r16, r16
	mov r24, r16
	jmp _L_main_epilogue
	; epilogue start
_L_main_epilogue:
	adiw r28, 0
	out SPH, r29
	out SPL, r28
	pop r29
	pop r28
	ret
