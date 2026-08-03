	.file	"test.c"
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__SREG__ = 0x3f
__tmp_reg__ = 0
__zero_reg__ = 1
	.text
.global	asd
	.type	asd, @function
asd:
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push r16
	push r17
	push r28
	push r29
	in r28,__SP_L__
	in r29,__SP_H__
	sbiw r28,18
	in __tmp_reg__,__SREG__
	cli
	out __SP_H__,r29
	out __SREG__,__tmp_reg__
	out __SP_L__,r28
/* prologue: function */
/* frame size = 18 */
/* stack size = 30 */
.L__stack_usage = 30
	std Y+2,r25
	std Y+1,r24
	std Y+4,r23
	std Y+3,r22
	std Y+6,r21
	std Y+5,r20
	std Y+8,r19
	std Y+7,r18
	std Y+10,r17
	std Y+9,r16
	std Y+12,r15
	std Y+11,r14
	std Y+14,r13
	std Y+13,r12
	std Y+16,r11
	std Y+15,r10
	std Y+18,r9
	std Y+17,r8
	ldd r24,Y+1
	ldd r25,Y+2
/* epilogue start */
	adiw r28,18
	in __tmp_reg__,__SREG__
	cli
	out __SP_H__,r29
	out __SREG__,__tmp_reg__
	out __SP_L__,r28
	pop r29
	pop r28
	pop r17
	pop r16
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	ret
	.size	asd, .-asd
.global	main
	.type	main, @function
main:
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push r16
	push r17
	push r28
	push r29
	rcall .
	in r28,__SP_L__
	in r29,__SP_H__
/* prologue: function */
/* frame size = 2 */
/* stack size = 14 */
.L__stack_usage = 14
	push __zero_reg__
	ldi r24,lo8(16)
	push r24
	push __zero_reg__
	ldi r24,lo8(15)
	push r24
	push __zero_reg__
	ldi r24,lo8(14)
	push r24
	push __zero_reg__
	ldi r24,lo8(13)
	push r24
	push __zero_reg__
	ldi r24,lo8(12)
	push r24
	push __zero_reg__
	ldi r24,lo8(11)
	push r24
	push __zero_reg__
	ldi r24,lo8(10)
	push r24
	mov __tmp_reg__,r31
	ldi r31,lo8(9)
	mov r8,r31
	mov r9,__zero_reg__
	mov r31,__tmp_reg__
	set
	clr r10
	bld r10,3
	mov r11,__zero_reg__
	mov __tmp_reg__,r31
	ldi r31,lo8(7)
	mov r12,r31
	mov r13,__zero_reg__
	mov r31,__tmp_reg__
	mov __tmp_reg__,r31
	ldi r31,lo8(6)
	mov r14,r31
	mov r15,__zero_reg__
	mov r31,__tmp_reg__
	ldi r16,lo8(5)
	ldi r17,0
	ldi r18,lo8(4)
	ldi r19,0
	ldi r20,lo8(3)
	ldi r21,0
	ldi r22,lo8(2)
	ldi r23,0
	ldi r24,lo8(1)
	ldi r25,0
	call asd
	in r18,__SP_L__
	in r19,__SP_H__
	subi r18,-14
	sbci r19,-1
	in __tmp_reg__,__SREG__
	cli
	out __SP_H__,r29
	out __SREG__,__tmp_reg__
	out __SP_L__,r28
	std Y+2,r25
	std Y+1,r24
	ldd r24,Y+1
	ldd r25,Y+2
/* epilogue start */
	pop __tmp_reg__
	pop __tmp_reg__
	pop r29
	pop r28
	pop r17
	pop r16
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	ret
	.size	main, .-main
	.ident	"GCC: (GNU) 7.3.0"
