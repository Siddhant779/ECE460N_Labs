    .ORIG x3000
    AND R0, R0, #0
    BRz next1
    HALT
next1
    ADD R0, R0, #1
    BRp next2
    HALT
next2
    ADD R0, R0, #-2
    BRn next3
    HALT
next3
    BRzp exit
    AND R0, R0, #0
    BRnp exit
    BRnzp next4
    HALT
next4
    ADD R0, R0, #10
    BRnz next1
    HALT
    .FILL x0000
    .FILL x0000
    .FILL x0000
    .FILL x0000
    .FILL x0000
    .FILL x0000
    .FILL x0000
    .FILL x0000
    .FILL x0000
    .FILL x0000
    .FILL x0000
    .FILL x0000
    .FILL x0000
    .FILL x0000
    .FILL x0000
    .FILL x0000
    .FILL x0000
    .FILL x0000
exit
    HALT
    .END