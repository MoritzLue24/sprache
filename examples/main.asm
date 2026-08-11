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
	ldi r16, 2
	neg r16
	ldi r17, 5
	neg r17
	mul r16, r17
	mov r17, r0
	ldi r18, 56
	add r17, r18
	ldi r18, 5
	ldi r19, 2
	mov r16, r18
	add r16, r19
	neg r16
	add r16, r17
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
