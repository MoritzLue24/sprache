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
horner:
	push r28
	push r29
	in r29, SPH
	in r28, SPL
	sbiw r28, 4
	out SPH, r29
	out SPL, r28
	; prologue end, stack size = 4
	ldi r16, 3
	ldd r17, Y+9
	mul r16, r17
	mov r17, r0
	std Y+1, r17
	ldd r17, Y+1
	ldi r18, 2
	add r17, r18
	std Y+2, r17
	ldd r17, Y+2
	ldd r18, Y+9
	mul r17, r18
	mov r17, r0
	std Y+3, r17
	ldd r17, Y+3
	ldi r18, 1
	add r17, r18
	std Y+4, r17
	ldd r16, Y+4
	mov r24, r16
	jmp _L_horner_epilogue
	; epilogue start
_L_horner_epilogue:
	adiw r28, 4
	out SPH, r29
	out SPL, r28
	pop r29
	pop r28
	ret
main:
	push r28
	push r29
	in r29, SPH
	in r28, SPL
	sbiw r28, 1
	out SPH, r29
	out SPL, r28
	; prologue end, stack size = 1
	ldi r17, 7
	push r17
	call horner
	mov r17, r24
	pop r3
	std Y+1, r17
	ldd r16, Y+1
	mov r24, r16
	jmp _L_main_epilogue
	; epilogue start
_L_main_epilogue:
	adiw r28, 1
	out SPH, r29
	out SPL, r28
	pop r29
	pop r28
	ret
