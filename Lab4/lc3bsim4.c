/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {
    IRD1, IRD0,
    COND2,COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_SSP,
    LD_USP,
    LD_PSR,
    LD_VECTOR,
    LD_EXEC,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_SP,
    GATE_PSR,
    GATE_TABLE,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    SPMUX1,SPMUX0,
    PSRMUX,
    DRMUX1,DRMUX0,
    SR1MUX1,SR1MUX0,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
/* MODIFY: you have to add all your new control signals */
    CONTROL_STORE_BITS
} CS_BITS;

enum excep {
    filler,
    interruptLC,
    protect,
    unaligned,
    opcode
}excep;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD1] <<1)+ (x[IRD0]); }
int GetCOND(int *x)          { return(x[COND2] << 2)+(x[COND1] << 1) +
                                     (x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
                                      (x[J3] << 3) + (x[J2] << 2) +
                                      (x[J1] << 1) + x[J0]); }
                                    
int GetLD_SSP(int *x)        { return(x[LD_SSP]); }
int GetLD_USP(int *x)        { return(x[LD_USP]); }
int GetLD_PSR(int *x)        { return(x[LD_PSR]); }
int GetLD_VECTOR(int *x)     { return(x[LD_VECTOR]); }
int GetLD_EXEC(int *x)       { return(x[LD_EXEC]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_SP(int *x)       { return(x[GATE_SP]); }
int GetGATE_PSR(int *x)      { return(x[GATE_PSR]); }
int GetGATE_TABLE(int *x)    { return(x[GATE_TABLE]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetSPMUX(int *x)         { return((x[SPMUX1] << 1) + x[SPMUX0]); }
int GetPSRMUX(int *x)        { return(x[PSRMUX]); }
int GetDRMUX(int *x)         { return((x[DRMUX1] << 1) + x[DRMUX0]); }
int GetSR1MUX(int *x)        { return((x[SR1MUX1] << 1) + x[SR1MUX0]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); }
int GetLSHF1(int *x)         { return(x[LSHF1]); }
/* MODIFY: you can add more Get functions for your new control signals */


/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

    int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

    int READY;	/* ready bit */
    /* The ready bit is also latched as you dont want the memory system to assert it
       at a bad point in the cycle*/

    int REGS[LC_3b_REGS]; /* register file. */

    int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

    int STATE_NUMBER; /* Current State Number - Provided for debugging */

/* For lab 4 */
    int VECT;
    int SSP; /* Initial value of system stack pointer */
    int USP;
/* MODIFY: You may add system latches that are required by your implementation */
    int PSR;
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

    eval_micro_sequencer();
    cycle_memory();
    eval_bus_drivers();
    drive_bus();
    latch_datapath_values();

    CURRENT_LATCHES = NEXT_LATCHES;

    CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
    int i;

    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
        if (CURRENT_LATCHES.PC == 0x0000) {
            RUN_BIT = FALSE;
            printf("Simulator halted\n\n");
            break;
        }
        cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {
    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
        cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
    int address; /* this is a byte address */

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
    int k;

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
        case 'G':
        case 'g':
            go();
            break;

        case 'M':
        case 'm':
            scanf("%i %i", &start, &stop);
            mdump(dumpsim_file, start, stop);
            break;

        case '?':
            help();
            break;
        case 'Q':
        case 'q':
            printf("Bye.\n");
            exit(0);

        case 'R':
        case 'r':
            if (buffer[1] == 'd' || buffer[1] == 'D')
                rdump(dumpsim_file);
            else {
                scanf("%d", &cycles);
                run(cycles);
            }
            break;

        default:
            printf("Invalid Command\n");
            break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
        printf("Error: Can't open micro-code file %s\n", ucode_filename);
        exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
        if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
            printf("Error: Too few lines (%d) in micro-code file: %s\n",
                   i, ucode_filename);
            exit(-1);
        }

        /* Put in bits one at a time. */
        index = 0;

        for (j = 0; j < CONTROL_STORE_BITS; j++) {
            /* Needs to find enough bits in line. */
            if (line[index] == '\0') {
                printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
                       ucode_filename, i);
                exit(-1);
            }
            if (line[index] != '0' && line[index] != '1') {
                printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
                       ucode_filename, i, j);
                exit(-1);
            }

            /* Set the bit in the Control Store. */
            CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
            index++;
        }

        /* Warn about extra bits in line. */
        if (line[index] != '\0')
            printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
                   ucode_filename, i);
    }
    printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
        MEMORY[i][0] = 0;
        MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
        printf("Error: Can't open program file %s\n", program_filename);
        exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
        program_base = word >> 1;
    else {
        printf("Error: Program file is empty\n");
        exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
        /* Make sure it fits. */
        if (program_base + ii >= WORDS_IN_MEM) {
            printf("Error: Program file %s is too long to fit in memory. %x\n",
                   program_filename, ii);
            exit(-1);
        }

        /* Write the word to memory array. */
        MEMORY[program_base + ii][0] = word & 0x00FF;
        MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
        ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *argv[], int num_prog_files) {
    int i;
    init_control_store(argv[1]);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
        load_program(argv[i + 2]);
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */

    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 3) {
        printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
               argv[0]);
        exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv, argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
        printf("Error: Can't open dumpsim file\n");
        exit(-1);
    }

    while (1)
        get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.

   Do not modify the rdump and mdump functions.

   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/

#define INSTR(pos, wid,instr) ((instr) >> (pos) & ~(~0<<(wid)))
#define Low8bits(x) ((x) & 0x00FF)
#define High8bits(x) ((x) & 0xFF00)
#define SIGN_EXT(num,wid) ((num) >> (wid) - 1? ~0 <<(wid) | (num) :(num))
#define ZERO_EXTEND(num,pos,wid) ((num) >> (pos) & ~(~0 << (wid)))
#define Table 0x0200

int inter_request = 0;
int value_gateSP=0;
int value_gatePSR=0;
int value_gateTable=0;

int first_cycle = 0;

int mem_cycle=0;
int value_gatePC=0;
int value_gateMDR=0;
int value_gateALU=0;
int value_gateMARMUX=0;
int value_gateSHF=0;
int main_addr_output=0;
int value_gatePCFinal=0;

void eval_micro_sequencer() {

    /*
     * Evaluate the address of the next state according to the
     * micro sequencer logic. Latch the next microinstruction.
     */
    if(first_cycle==0){
        CURRENT_LATCHES.PSR = 0x8000;
        first_cycle=1;
    }
    if(CYCLE_COUNT==300){
        inter_request=1;
    }
    int state = CURRENT_LATCHES.STATE_NUMBER;
    int IRD = GetIRD(CURRENT_LATCHES.MICROINSTRUCTION);
    int j = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
    int cond = GetCOND(CURRENT_LATCHES.MICROINSTRUCTION);
    int add_mode = INSTR(11,1,CURRENT_LATCHES.IR);
    int next_instr[CONTROL_STORE_BITS];
    if(IRD==1){
        int next_state = INSTR(12,4,CURRENT_LATCHES.IR);
        for(int i=0;i<CONTROL_STORE_BITS;i++){
            next_instr[i] = CONTROL_STORE[next_state][i];
        }
        memcpy(NEXT_LATCHES.MICROINSTRUCTION,next_instr,sizeof(int)*CONTROL_STORE_BITS);
        NEXT_LATCHES.STATE_NUMBER = next_state;
        return;
    }
    else if(IRD==0){
        if(cond==0){
            for(int i=0;i<CONTROL_STORE_BITS;i++){
                next_instr[i] = CONTROL_STORE[j][i];
            }
            memcpy(NEXT_LATCHES.MICROINSTRUCTION,next_instr,sizeof(int)*CONTROL_STORE_BITS);
            NEXT_LATCHES.STATE_NUMBER = j;
            return;
        }
        if(cond==1){
            if(CURRENT_LATCHES.READY==1){
                j+=2;
                NEXT_LATCHES.READY=0;
            }
            for(int i=0;i<CONTROL_STORE_BITS;i++){
                next_instr[i] = CONTROL_STORE[j][i];
            }
            memcpy(NEXT_LATCHES.MICROINSTRUCTION,next_instr,sizeof(int)*CONTROL_STORE_BITS);
            NEXT_LATCHES.STATE_NUMBER = j;
            return;
        }
        if(cond==2){
            if(CURRENT_LATCHES.BEN==1){
                j+=4;
            }
            for(int i=0;i<CONTROL_STORE_BITS;i++){
                next_instr[i] = CONTROL_STORE[j][i];
            }
            memcpy(NEXT_LATCHES.MICROINSTRUCTION,next_instr,sizeof(int)*CONTROL_STORE_BITS);
            NEXT_LATCHES.STATE_NUMBER = j;
            return;
        }
        if(cond==3){
            if(add_mode==1){
                j+=1;
            }
            for(int i=0;i<CONTROL_STORE_BITS;i++){
                next_instr[i] = CONTROL_STORE[j][i];
            }
            memcpy(NEXT_LATCHES.MICROINSTRUCTION,next_instr,sizeof(int)*CONTROL_STORE_BITS);
            NEXT_LATCHES.STATE_NUMBER = j;
            return;
        }
        if(cond==4){
            int jsr15 = INSTR(15,1,CURRENT_LATCHES.PSR);
            if(jsr15==1){
                j+=8;
            }
            for(int i=0;i<CONTROL_STORE_BITS;i++){
                next_instr[i] = CONTROL_STORE[j][i];
            }
            memcpy(NEXT_LATCHES.MICROINSTRUCTION,next_instr,sizeof(int)*CONTROL_STORE_BITS);
            NEXT_LATCHES.STATE_NUMBER = j;
            return;
        }

    }


}


void cycle_memory() {

    /*
     * This function emulates memory and the WE logic.
     * Keep track of which cycle of MEMEN we are dealing with.
     * If fourth, we need to latch Ready bit at the end of
     * cycle to prepare microsequencer for the fifth cycle.
     */
    int mem_use = GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION);
    if(mem_use==1){
        mem_cycle++;
    }
    else{
        return;
    }
}



void eval_bus_drivers() {

    /*
     * Datapath routine emulating operations before driving the bus.
     * Evaluate the input of tristate drivers
     *             Gate_MARMUX,
     *		 Gate_PC,
     *		 Gate_ALU,
     *		 Gate_SHF,
     *		 Gate_MDR.
     */
    int spMux = GetSPMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    int psrMux = GetPSRMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    int psrMux_output = 0;
    int data_size = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
    int updated_mar = CURRENT_LATCHES.MAR >> 1;
    if(data_size==0){
        int mar_zero = INSTR(0,1,CURRENT_LATCHES.MAR);
        if(mar_zero==0){
            value_gateMDR=Low16bits(SIGN_EXT((MEMORY[updated_mar][0]),8));
        }
        if(mar_zero==1){
            value_gateMDR = Low16bits(SIGN_EXT((MEMORY[updated_mar][1]),8));
        }
    }
    if(data_size==1){
        value_gateMDR = CURRENT_LATCHES.MDR;
    }
    int dr_mux = GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    int immm5 = INSTR(0,5,CURRENT_LATCHES.IR);
    int am4 = INSTR(0,4,CURRENT_LATCHES.IR);
    int off6 = INSTR(0,6,CURRENT_LATCHES.IR);
    int off9 = INSTR(0,9,CURRENT_LATCHES.IR);
    int off11 = INSTR(0,11,CURRENT_LATCHES.IR);
    int sr2mux_output;
    int bit5= INSTR(5,1,CURRENT_LATCHES.IR);
    if(bit5==0){
        sr2mux_output = CURRENT_LATCHES.REGS[INSTR(0,3,CURRENT_LATCHES.IR)];
    }
    else if(bit5==1){
        sr2mux_output = SIGN_EXT(immm5,5);
    }
    int sr1mux = GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    int addr1mux = GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    int addr2mux = GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    int marmux = GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    int aluk = GetALUK(CURRENT_LATCHES.MICROINSTRUCTION);
    int pcmux = GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    int lshf = GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION);
    int sr1mux_output;
    int dr1mux_output;
    int addr1mux_output;
    int addr2mux_output;
    if(sr1mux==0){
        int reg_num = INSTR(9,3,CURRENT_LATCHES.IR);
        sr1mux_output = CURRENT_LATCHES.REGS[reg_num];
    }
    else if(sr1mux==1){
        int reg_num = INSTR(6,3,CURRENT_LATCHES.IR);
        sr1mux_output = CURRENT_LATCHES.REGS[reg_num];
    }
    else if(sr1mux==2){
        sr1mux_output = CURRENT_LATCHES.REGS[6];
    }
    if(dr_mux==0){
        int reg_num = INSTR(9,3,CURRENT_LATCHES.IR);
        dr1mux_output = CURRENT_LATCHES.REGS[reg_num];
    }
    else if(dr_mux==1){
        dr1mux_output = CURRENT_LATCHES.REGS[7];
    }
    else if(dr_mux==2){
        dr1mux_output = CURRENT_LATCHES.REGS[6];
    }
    if(addr1mux==0){
        addr1mux_output= CURRENT_LATCHES.PC;
    }
    else if(addr1mux==1){
        addr1mux_output = sr1mux_output;
    }
    //sp mux is after this
    if(spMux==0){
        value_gateSP= CURRENT_LATCHES.SSP;
    }
    if(spMux==1){
        value_gateSP= sr1mux_output-2;
    }
    if(spMux==2){
        value_gateSP= sr1mux_output+2;
    }
    if(spMux==3){
        value_gateSP= CURRENT_LATCHES.USP;
    }
    if(addr2mux==0){
        addr2mux_output=0;
    }
    if(addr2mux==1){
        addr2mux_output= SIGN_EXT(off6,6);
    }
    if(addr2mux==2){
        addr2mux_output= SIGN_EXT(off9,9);
    }
    if(addr2mux==3){
        addr2mux_output= SIGN_EXT(off11,11);
    }
    if(lshf==1){
        addr2mux_output = addr2mux_output<<1;
    }
    main_addr_output= addr2mux_output+addr1mux_output;
    if(aluk==0){
        value_gateALU = sr2mux_output+sr1mux_output;
    }
    else if(aluk==1){
        value_gateALU = sr2mux_output & sr1mux_output;
    }
    else if(aluk==2){
        value_gateALU = sr2mux_output ^ sr1mux_output;
    }
    else if(aluk==3){
        if(data_size==1){
            value_gateALU = sr1mux_output;
        }
        else if(data_size==0){
            value_gateALU = (Low8bits(sr1mux_output)<<8)+(Low8bits(sr1mux_output));
        }
    }
    int shift= INSTR(4, 2,CURRENT_LATCHES.IR);
    if(shift==0){
        value_gateSHF= sr1mux_output<<am4;
    }
    else if(shift==1){
        value_gateSHF = sr1mux_output>>am4;
    }
    else if(shift==3){
        value_gateSHF = (SIGN_EXT(sr1mux_output, 16) >> am4);
    }
    if(marmux==0){
        int ir7 = INSTR(0,8,CURRENT_LATCHES.IR);
        value_gateMARMUX= (ZERO_EXTEND(ir7,0,8))<<1;
    }
    else if(marmux==1){
        value_gateMARMUX = main_addr_output;
    }
    value_gatePCFinal=CURRENT_LATCHES.PC;
    if(pcmux==0){
        value_gatePC = CURRENT_LATCHES.PC;
    }
    else if(pcmux==1){
        value_gatePC = BUS;
    }
    else if(pcmux==2){
        value_gatePC = main_addr_output;
    }
    else if(pcmux==3){
        value_gatePCFinal=CURRENT_LATCHES.PC-2;
    }
    value_gatePSR = CURRENT_LATCHES.PSR;
    value_gateTable = CURRENT_LATCHES.VECT + Table;
}


void drive_bus() {

    /*
     * Datapath routine for driving the bus from one of the 5 possible
     * tristate drivers.
     */
    int GateTable = GetGATE_TABLE(CURRENT_LATCHES.MICROINSTRUCTION);
    int GatePSR = GetGATE_PSR(CURRENT_LATCHES.MICROINSTRUCTION);
    int GateSP = GetGATE_SP(CURRENT_LATCHES.MICROINSTRUCTION);
    int Gatepc = GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION);
    int Gatemdr = GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION);
    int Gatealu = GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION);
    int GateMarmux = GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    int Gateshf = GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION);
    if(Gatepc || Gatemdr || Gatealu || GateMarmux || Gateshf || GateSP || GatePSR || GateTable) {
        if(GateTable){
            BUS = Low16bits(value_gateTable);
        }
        if(GateSP) {
            BUS = Low16bits(value_gateSP);
        }
        if(GatePSR) {
            BUS = Low16bits(value_gatePSR);
        }
        if (Gatepc) {
            BUS = Low16bits(value_gatePCFinal);
        }
        if (Gatemdr) {
            BUS = Low16bits(value_gateMDR);
        }
        if (Gatealu) {
            BUS = Low16bits(value_gateALU);
        }
        if (GateMarmux) {
            BUS = Low16bits(value_gateMARMUX);
        }
        if (Gateshf) {
            BUS = Low16bits(value_gateSHF);
        }
    }
    else{
        BUS= Low16bits(0);
    }

}


void latch_datapath_values() {

    /*
     * Datapath routine for computing all functions that need to latch
     * values in the data path at the end of this cycle.  Some values
     * require sourcing the bus; therefore, this routine has to come
     * after drive_bus.
     */

    // do psr mux and latching for psr here
    int PSR_changed = 0;
    int ld_psr = GetLD_PSR(CURRENT_LATCHES.MICROINSTRUCTION);
    int ld_exc = GetLD_EXEC(CURRENT_LATCHES.MICROINSTRUCTION);
    int ld_ssp = GetLD_SSP(CURRENT_LATCHES.MICROINSTRUCTION);
    int ld_usp = GetLD_USP(CURRENT_LATCHES.MICROINSTRUCTION);
    int ld_vector = GetLD_VECTOR(CURRENT_LATCHES.MICROINSTRUCTION);
    if(mem_cycle==MEM_CYCLES-1){
        NEXT_LATCHES.READY=1;
    }
    int ld_mar = GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION);
    int ld_mdr = GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION);
    int ld_ir = GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION);
    int ld_ben = GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION);
    int ld_reg = GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION);
    int ld_cc = GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION);
    int ld_pc = GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION);
    int dr_mux = GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    if(ld_psr){
        int psrMux = GetPSRMUX(CURRENT_LATCHES.MICROINSTRUCTION);
        if(psrMux==0){
            NEXT_LATCHES.PSR = CURRENT_LATCHES.PSR & 0x7FFF;
            PSR_changed=1;
        }
        if(psrMux==1){
            NEXT_LATCHES.PSR = BUS;
            int n  = INSTR(2,1,NEXT_LATCHES.PSR);
            int z  = INSTR(1,1,NEXT_LATCHES.PSR);
            int p  = INSTR(0,1,NEXT_LATCHES.PSR);
            PSR_changed=1;
            if(z){
                NEXT_LATCHES.Z=1;
                NEXT_LATCHES.P=0;
                NEXT_LATCHES.N=0;
            }
            else if(n){
                NEXT_LATCHES.Z=0;
                NEXT_LATCHES.P=0;
                NEXT_LATCHES.N=1;
            }
            else if(p){
                NEXT_LATCHES.Z=0;
                NEXT_LATCHES.P=1;
                NEXT_LATCHES.N=0;
            }

        }
    }
    if(ld_ssp){
        int sr1Mux = GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
        int sr = INSTR(9,3,CURRENT_LATCHES.IR);
        if(sr1Mux==1){
            sr = INSTR(6,3,CURRENT_LATCHES.IR);
        }
        if(sr1Mux==2){
            sr=6;
        }
        NEXT_LATCHES.SSP = CURRENT_LATCHES.REGS[sr];
    }
    if(ld_usp){
        int sr1Mux = GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
        int sr = INSTR(9,3,CURRENT_LATCHES.IR);
        if(sr1Mux==1){
            sr = 7;
        }
        if(sr1Mux==2){
            sr=6;
        }
        NEXT_LATCHES.USP = CURRENT_LATCHES.REGS[sr];
    }
    if(ld_vector){
        NEXT_LATCHES.VECT = CURRENT_LATCHES.VECT<<1;
    }
    if(ld_mar){
        NEXT_LATCHES.MAR = BUS;
    }
    if(ld_mdr){
        NEXT_LATCHES.MDR = BUS;
    }
    if(ld_ir){
        NEXT_LATCHES.IR = BUS;
    }
    if(ld_ben){
        int n = CURRENT_LATCHES.N;
        int z = CURRENT_LATCHES.Z;
        int p = CURRENT_LATCHES.P;
        int ir_n = INSTR(11,1,CURRENT_LATCHES.IR);
        int ir_z = INSTR(10,1,CURRENT_LATCHES.IR);
        int ir_p = INSTR(9,1,CURRENT_LATCHES.IR);
        NEXT_LATCHES.BEN = (n & ir_n) + (z & ir_z) + (p & ir_p);
    }
    if(ld_reg){
        int dr = INSTR(9,3,CURRENT_LATCHES.IR);
        if(dr_mux==1){
            dr=7;
        }
        if(dr_mux==2){
            dr=6;
        }
        NEXT_LATCHES.REGS[dr]= BUS;
    }
    if(ld_cc){
        int sign_bit= INSTR(15,1,BUS);
        if(BUS==0){
            NEXT_LATCHES.Z=1;
            NEXT_LATCHES.P=0;
            NEXT_LATCHES.N=0;
        }
        else if(sign_bit){
            NEXT_LATCHES.Z=0;
            NEXT_LATCHES.P=0;
            NEXT_LATCHES.N=1;
        }
        else if(sign_bit==0){
            NEXT_LATCHES.Z=0;
            NEXT_LATCHES.P=1;
            NEXT_LATCHES.N=0;
        }

    }
    if(ld_pc){
        int pc_mux = GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION);
        if(pc_mux==0){
            NEXT_LATCHES.PC = CURRENT_LATCHES.PC+2;
        }
        else if(pc_mux==1){
            NEXT_LATCHES.PC = BUS;
        }
        else if(pc_mux==2){
            NEXT_LATCHES.PC = main_addr_output;
        }
    }
    if(mem_cycle==MEM_CYCLES) {
        mem_cycle=0;
        int state = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
        int read_write = GetR_W(CURRENT_LATCHES.MICROINSTRUCTION);
        int data_size = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
        int updated_mar = CURRENT_LATCHES.MAR >> 1;
        int current_mdr = CURRENT_LATCHES.MDR;
        if (read_write) {
            if (data_size) {
                //state 16
                int msb_md = (current_mdr & 0xFF00)>>8;
                int lsb_md = current_mdr & 0x00FF;
                MEMORY[updated_mar][0] = lsb_md;
                MEMORY[updated_mar][1] = msb_md;
            } else {
                //state 17
                int mar_zero = INSTR(0, 1, CURRENT_LATCHES.MAR);
                MEMORY[updated_mar][mar_zero] = Low8bits(current_mdr);
            }
        } else {
            if (ld_mdr==1) {
                //state 25,28,29,33
                NEXT_LATCHES.MDR = Low16bits(
                        (Low8bits(MEMORY[updated_mar][0])) + High8bits((MEMORY[updated_mar][1]) << 8));
            }
        }
    }
    int interr  = inter_request && CURRENT_LATCHES.STATE_NUMBER==18;
    int exception_caused = 0;
    int next_instr[CONTROL_STORE_BITS];
    int next_MAR_value  = NEXT_LATCHES.MAR;
    int IRD = GetIRD(CURRENT_LATCHES.MICROINSTRUCTION);
    int J = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
    int psr15 = INSTR(15,1,CURRENT_LATCHES.PSR);
    if(NEXT_LATCHES.STATE_NUMBER==10 || NEXT_LATCHES.STATE_NUMBER==11){
        exception_caused=1;
        NEXT_LATCHES.VECT = opcode;
        for(int i=0;i<CONTROL_STORE_BITS;i++){
            next_instr[i] = CONTROL_STORE[54][i];
            }
            memcpy(NEXT_LATCHES.MICROINSTRUCTION,next_instr,sizeof(int)*CONTROL_STORE_BITS);
            NEXT_LATCHES.STATE_NUMBER = 54;
    }
    if(IRD==2){
         if(0<=next_MAR_value && next_MAR_value<=0x2FFF && psr15==1){
            exception_caused=1;
            NEXT_LATCHES.VECT = protect;
         }
        int unaligned_check = INSTR(0,1,NEXT_LATCHES.MAR);
        int next_j = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
        for(int i=0;i<CONTROL_STORE_BITS;i++){
            next_instr[i] = CONTROL_STORE[next_j][i];
            }
        int testing_off = 0;
        if(GetDATA_SIZE(next_instr) && unaligned_check && exception_caused==0){
            exception_caused=1;
            NEXT_LATCHES.VECT  = unaligned;
        }
        if(interr==1 && exception_caused==0){

            for(int i=0;i<CONTROL_STORE_BITS;i++){
            next_instr[i] = CONTROL_STORE[38][i];
            }
            memcpy(NEXT_LATCHES.MICROINSTRUCTION,next_instr,sizeof(int)*CONTROL_STORE_BITS);
            NEXT_LATCHES.STATE_NUMBER = 38;
            NEXT_LATCHES.VECT = interruptLC<<1;
            inter_request=0;
        }
        if(exception_caused==1){
            for(int i=0;i<CONTROL_STORE_BITS;i++){
            next_instr[i] = CONTROL_STORE[54][i];
            }
            memcpy(NEXT_LATCHES.MICROINSTRUCTION,next_instr,sizeof(int)*CONTROL_STORE_BITS);
            NEXT_LATCHES.STATE_NUMBER = 54;
         }
        if(exception_caused==0 && interr==0){
            for(int i=0;i<CONTROL_STORE_BITS;i++){
                next_instr[i] = CONTROL_STORE[J][i];
            }
            memcpy(NEXT_LATCHES.MICROINSTRUCTION,next_instr,sizeof(int)*CONTROL_STORE_BITS);
            NEXT_LATCHES.STATE_NUMBER = J;        
        }

    }
    if(PSR_changed==0){
        if(NEXT_LATCHES.Z==1){
            NEXT_LATCHES.PSR = (CURRENT_LATCHES.PSR | 0x0002) & 0xFFF2;
        }
        if(NEXT_LATCHES.P==1){
            NEXT_LATCHES.PSR = (CURRENT_LATCHES.PSR | 0x0001) & 0xFFF1;
        }
        if(NEXT_LATCHES.N==1){
            NEXT_LATCHES.PSR = (CURRENT_LATCHES.PSR | 0x0004) & 0xFFF4;
        }
    }
}