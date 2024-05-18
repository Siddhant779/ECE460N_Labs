.orig x3000

ADD R1,R2,R3
ADD R1,R2,R3    
ADD R1,R2,R3 
ADD R1,R2,R3    
ADD R1,R2,R3  
ADD R1, R2, x5   
ADD R1, R1, x-5   
ADD R1, R1, x-1        
AND R0, R5, R6   
AND R3, R2, xA       
XOR R1, R2, R3   
XOR R1, R2, xa   
NOT R1, R1       
NOT R7, R7       
TRAP x25               
TRAP x25            
TRAP x25         
STW R7, R6, #0  
STW R7, R6, #0      
STW R7, R6, #0  
STB R7, R6, #0   
STB R7, R6, #0   
STB R7, R6, #0         
STB R7, R6, #1   
STB R7, R6, #1   
STB R7, R6, #1   
LDW R0, R6, #0           
LDW R0, R6, #0           
LDW R0, R6, #0           
LDB R0, R6, #0           
LDB R0, R6, #0           
LDB R0, R6, #0           
LDB R0, R6, #1           
LDB R0, R6, #1           
LDB R0, R6, #1          
LEA R2, label           
JMP R2             
JMP R2           
JSRR R2           
JSRR R2                  
JSRR R2                 
JSR  label            
JSR  label               
RET                  
LSHF R2, R3,#4         
RSHFL R2, R6, #4         
RSHFA R2, R6, #4        
BRn label    
       
.end