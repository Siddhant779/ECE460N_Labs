	.orig x3000
start	lea   r6 stack
	ldw   r6 r6 #0
	jsr   main
	halt

main	stw   r7 r6 #-1
	add   r6 r6 #-2
	jsr   sum
	lea   r3 copy
	jsrr  r3
	jsr   misc
	ldw   r7 r6 #0
	add   r6 r6 #2
	ret

sum	lea   r0 arr
	lea   r3 arre
	not   r3 r3
	add   r3 r3 #-1
	and   r2 r2 #0
sloop	ldw   r1 r0 #0
	add   r2 r2 r1
	add   r0 r0 #2
	add   r1 r0 r3
	brnp  sloop
	lea   r0 arrsum
	stw   r2 r0 #0
	ret

copy	lea   r0 src
	lea   r1 dst
cloop	ldb   r2 r0 #0
	brz   cend
	stb   r2 r1 #0
	add   r0 r0 #1
	add   r1 r1 #1
	br    cloop
cend	ret

misc	stw   r7 r6 #-1
	add   r6 r6 #-2
	lea   r0 struct
	and   r4 r4 #0
	add   r4 r4 #1
	
	ldw   r1 r0 #0
	ldw   r2 r0 #1
	and   r3 r1 r2
	xor   r3 r2 r3
	add   r2 r1 r3 ; r2 = w0 OR w1
	ldw   r1 r0 #2
	jsr   flag

	ldw   r1 r0 #3
	ldw   r2 r0 #4
	add   r2 r1 r2 ; r2 = w3 + w4
	ldw   r1 r0 #5
	jsr   flag

	ldw   r1 r0 #3
	rshfa r2 r1 #2 ; r2 = w3 >>> 2
	ldw   r1 r0 #6
	jsr   flag

	ldw   r1 r0 #7
	rshfa r2 r1 #3 ; r2 = w6 >>> 3
	ldw   r1 r0 #8
	jsr   flag

	ldw   r1 r0 #7
	lshf  r2 r1 #2 ; r2 = w6 << 2
	ldw   r1 r0 #9
	jsr   flag

	ldw   r1 r0 #7
	rshfl r2 r1 #4 ; r2 = w6 >> 4
	ldw   r1 r0 #10
	jsr   flag

	ldw   r7 r6 #0
	add   r6 r6 #2
	ret

flag	not   r2 r2
	add   r2 r2 #1
	add   r1 r1 r2
	brnp  fskip
	ldb   r1 r0 #22
	add   r1 r1 r4
	stb   r1 r0 #22
fskip	lshf  r4 r4 #1
	ret

arr	.fill #1
	.fill #7
	.fill #0
	.fill #5
	.fill #2
arre	.fill #4

arrsum	.fill #0 ; sum = 19

src	.fill x6548 ; "Hello"
	.fill x6C6C
	.fill x006F

dst	.fill x0000
	.fill x0000
	.fill x0000

struct	.fill x5F61 ; word 0
	.fill x1F11 ; word 1
	.fill x5F71 ; word 2  ; w0 OR w1
	.fill #423  ; word 3
	.fill #158  ; word 4
	.fill #581  ; word 5  ; w3 + w4
	.fill #105  ; word 6  ; w3 >>> 2
	.fill #-184 ; word 7
	.fill #-23  ; word 8  ; w6 >>> 3
	.fill #-736 ; word 9  ; w6 << 2
	.fill x0FF4 ; word 10 ; w6 >> 4
	.fill x0000 ; byte 22 ; success flags

stack	.fill x8000
	
	.end