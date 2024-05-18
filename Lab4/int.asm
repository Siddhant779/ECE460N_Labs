.orig x1200

    add r6, r6, #-2 ; storing the registers that were used to not corrupt their values -push them on the stack
    stw r0,r6,#0 ; pushing r0 on the supervisor stack

    add r6, r6, #-2 
    stw r1,r6,#0 ; pushing r1 on the supervisor stack

    add r6, r6, #-2
    stw r2,r6,#0 ; pushing r2 on the supervisor stack

    add r6, r6, #-2
    stw r3,r6,#0 ; pushing r3 on the supervisor stack

    add r6, r6, #-2
    stw r4,r6,#0 ; pushing r4 on the supervisor stack

    add r6, r6, #-2
    stw r5,r6,#0 ; pushing r5 on the supervisor stack

    add r6, r6, #-2
    stw r7,r6,#0 ; pushing r7 on the supervisor stack


    lea r0, start
    ldw r0,r0, #0 ; r0 has 4000 now 
    ldb r1,r0, #0 ; r1 has the byte at 4000 now 
    add r1, r1, #1
    stb r1, r0, #0


    ldw r7,r6, #0 ; this is for popping the values back into the registers - popping r7 out of the stack
    add r6, r6, #2 

    ldw r5,r6, #0 ;popping r5 out of the stack
    add r6, r6, #2

    ldw r4,r6, #0 ; popping r4 out of the stack
    add r6, r6, #2
    
    ldw r3,r6, #0 ; popping r3 out of the stack 
    add r6, r6, #2

    ldw r2,r6, #0 ; popping r2 out of the stack
    add r6, r6, #2

    ldw r1,r6, #0 ; popping r1 out of the stack 
    add r6, r6, #2

    ldw r0,r6, #0 ; popping r0 out of the stack
    add r6, r6, #2


    rti
    halt 
start .fill x4000
.end