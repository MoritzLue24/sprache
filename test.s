	.file	"test.c"
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__SREG__ = 0x3f
__tmp_reg__ = 0
__zero_reg__ = 1
	.text
.global	main
	.type	main, @function
main:
	push r28
	push r29
	rcall .
	rcall .
	in r28,__SP_L__
	in r29,__SP_H__
/* prologue: function */
/* frame size = 4 */
/* stack size = 6 */
.L__stack_usage = 6
	ldi r24,lo8(2)
	ldi r25,0
	std Y+2,r25
	std Y+1,r24
	ldi r24,lo8(3)
	ldi r25,0
	std Y+4,r25
	std Y+3,r24
	ldd r18,Y+1
	ldd r19,Y+2
	ldd r24,Y+3
	ldd r25,Y+4
	mul r18,r24
	movw r20,r0
	mul r18,r25
	add r21,r0
	mul r19,r24
	add r21,r0
	clr r1
	movw r24,r20
/* epilogue start */
	pop __tmp_reg__
	pop __tmp_reg__
	pop __tmp_reg__
	pop __tmp_reg__
	pop r29
	pop r28
	ret
	.size	main, .-main
	.ident	"GCC: (GNU) 7.3.0"
