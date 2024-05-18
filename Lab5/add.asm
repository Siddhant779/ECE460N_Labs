.orig x3000

     
    ;add r6, r6, x0F - tesing the stack
    ;for testing the unknown code exception
    ;.fill xB000
    ;.fill xA000
    ;initialize the usp to 0xFE00
    lea r6, usp
    ldw r6,r6,#0 ; initializes r6 to 0xFE00 for the stack pointer 
    lea r0, first ; location of the first value
    ldw r0, r0, #0; r0 has 0xC000 now - r0 holds the address 

    and r2, r2, #0 ; will hold the sum 
    lea r3, iterat
    ldw r3,r3,#0 ; will hold the counter for the loop
loop ldb r1, r0, #0
    add r2, r1, r2
    add r0, r0, x0001
    add r3, r3, #-1 
    brp loop
    lea r0, store
    ldw r0, r0, #0
    stw r2, r0, #0 ; storing normally into 0xc014
    jmp r2 ; jumps to the address given by the value of the sum 

    ;lea r0, storePe ; this is for protection exception
    ;ldw r0, r0, #0
    ;stw r2, r0, #0 ; storing normally into 0x0000

    ;lea r0, storeUA ; this is for unaligned access exception
    ;ldw r0, r0, #0
    ;stw r2, r0, #0 ; storing normally into 0xc017

    
    halt 


init    .fill x4000
first   .fill xC000 
store   .fill xC014
storePe .fill x0000
storeUA .fill xC017
iterat  .fill #20
usp     .fill xFE00
.end 