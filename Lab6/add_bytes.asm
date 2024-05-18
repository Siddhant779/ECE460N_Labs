.orig x3000

    lea   r0, hi
    jmp   r0
    and   r2 r2 #0
    and   r0 r0 #0
hi	lea   r0, bytes  ; load r0 with pointer to bytes
    lea   r1, value
    ldw   r1, r1, #0
	ldw   r0, r0, #0
	stw   r1, r0, #0
	ldb   r1, r0, #0 ; load r1 and r2 with bytes
	ldb   r2, r0, #1

	add   r1, r1, r2 ; add bytes
	stb   r1, r0, #2 ; store the result

	brzp  nonNeg     ; if negative, take ones complement
	not   r1, r1
nonNeg	rshfl r2, r1, #7 ; overflow flag is in bit 7
	stb   r2, r0, #3 ; store overflow flag
	halt

bytes	.fill x3100      ; address of bytes
value .fill x8080 //7f7f and 8080 are overflowing 
	.end
