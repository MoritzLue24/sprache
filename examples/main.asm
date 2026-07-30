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
	sbiw r28, 12
	out SPH, r29
	out SPL, r28
	; prologue end, stack size = 12
	ldi r16, 2
	std Y+1, r16
	ldi r16, 3
	std Y+2, r16
	ldi r16, 4
	std Y+3, r16
	ldi r16, 5
	std Y+4, r16
	ldi r16, 6
	std Y+5, r16
	ldi r16, 7
	std Y+6, r16
	ldi r16, 8
	std Y+7, r16
	ldi r16, 9
	std Y+8, r16
	ldi r16, 10
	std Y+9, r16
	ldi r16, 11
	std Y+10, r16
	ldi r16, 12
	std Y+11, r16
	ldd r16, Y+1
	ldd r17, Y+2
	add r16, r17
	ldd r17, Y+3
	add r16, r17
	ldd r17, Y+4
	add r16, r17
	ldd r17, Y+5
	add r16, r17
	ldd r17, Y+6
	add r16, r17
	ldd r17, Y+7
	add r16, r17
	ldd r17, Y+8
	add r16, r17
	ldd r17, Y+9
	add r16, r17
	ldd r17, Y+10
	add r16, r17
	ldd r17, Y+11
	add r16, r17
	mov r24, r16
	jmp _L_main_epilogue
	; epilogue start
_L_main_epilogue:
	adiw r28, 12
	out SPH, r29
	out SPL, r28
	pop r29
	pop r28
	ret
