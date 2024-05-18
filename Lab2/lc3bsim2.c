// this is just for testing the file

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

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
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

    int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
    int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3b ISIM Help-----------------------\n");
    printf("go               -  run program to completion         \n");
    printf("run n            -  execute program for n instructions\n");
    printf("mdump low high   -  dump memory from low to high      \n");
    printf("rdump            -  dump the register & bus values    \n");
    printf("?                -  display this help menu            \n");
    printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

    process_instruction();
    CURRENT_LATCHES = NEXT_LATCHES;
    INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
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
/* Purpose   : Simulate the LC-3b until HALTed                 */
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

    printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
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
    printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
    printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
    fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
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

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) {
    int i;

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
        load_program(program_filename);
        while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
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
    if (argc < 2) {
        printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
               argv[0]);
        exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argc - 1);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
        printf("Error: Can't open dumpsim file\n");
        exit(-1);
    }

    while (1)
        get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/
void add(int code){
    int flag= ((code & 0x0020)>>5);
    int dr = ((code & 0x0E00)>>9);
    int sr1 = ((code & 0x01C0)>>6);
    if(flag==0){
        int sr2 =(code & 0x0007);
        int answer  = CURRENT_LATCHES.REGS[sr1]+CURRENT_LATCHES.REGS[sr2];
        int sign_answer= Low16bits(answer);
        NEXT_LATCHES.REGS[dr]= sign_answer;
        if(sign_answer==0){
            NEXT_LATCHES.Z=TRUE;
            NEXT_LATCHES.P=FALSE;
            NEXT_LATCHES.N=FALSE;
        }
        int sign_mem = (sign_answer & 0x8000) >>15;
        if(sign_mem){
            NEXT_LATCHES.Z=FALSE;
            NEXT_LATCHES.P=FALSE;
            NEXT_LATCHES.N=TRUE;
        }
        if(sign_mem==0){
            NEXT_LATCHES.Z=FALSE;
            NEXT_LATCHES.P=TRUE;
            NEXT_LATCHES.N=FALSE;
        }
    }
    else{
        int imm5= (code & 0x001F);
        int sign = (code & 0x0010)>>4;
        int extend_imm = sign ? imm5 | 0xFFE0 : imm5 & 0x001F;
        int answer= CURRENT_LATCHES.REGS[sr1] +extend_imm;
        int sign_answer= Low16bits(answer);
        NEXT_LATCHES.REGS[dr]= sign_answer;
        if(sign_answer==0){
            NEXT_LATCHES.Z=TRUE;
            NEXT_LATCHES.P=FALSE;
            NEXT_LATCHES.N=FALSE;
        }
        int sign_mem = (sign_answer & 0x8000) >>15;
        if(sign_mem){
            NEXT_LATCHES.Z=FALSE;
            NEXT_LATCHES.P=FALSE;
            NEXT_LATCHES.N=TRUE;
        }
        if(sign_mem==0){
            NEXT_LATCHES.Z=FALSE;
            NEXT_LATCHES.P=TRUE;
            NEXT_LATCHES.N=FALSE;
        }
    }
}
void and(int code){
    int flag= ((code & 0x0020)>>5);
    int dr = ((code & 0x0E00)>>9);
    int sr1 = ((code & 0x01C0)>>6);
    if(flag==0){
        int sr2 =(code & 0x0007);
        int answer  = CURRENT_LATCHES.REGS[sr1] & CURRENT_LATCHES.REGS[sr2];
        int sign_answer= Low16bits(answer);
        NEXT_LATCHES.REGS[dr]= sign_answer;
        if(sign_answer==0){
            NEXT_LATCHES.Z=TRUE;
            NEXT_LATCHES.P=FALSE;
            NEXT_LATCHES.N=FALSE;
        }
        int sign_mem = (sign_answer & 0x8000) >>15;
        if(sign_mem){
            NEXT_LATCHES.Z=FALSE;
            NEXT_LATCHES.P=FALSE;
            NEXT_LATCHES.N=TRUE;
        }
        if(sign_mem==0){
            NEXT_LATCHES.Z=FALSE;
            NEXT_LATCHES.P=TRUE;
            NEXT_LATCHES.N=FALSE;
        }
    }
    else{
        int imm5= (code & 0x001F);
        int sign = (code & 0x0010)>>4;
        int extend_imm = sign ? imm5 | 0xFFE0 : imm5 & 0x001F;
        int answer= CURRENT_LATCHES.REGS[sr1] & extend_imm;
        int sign_answer= Low16bits(answer);
        NEXT_LATCHES.REGS[dr]= sign_answer;
        if(sign_answer==0){
            NEXT_LATCHES.Z=TRUE;
            NEXT_LATCHES.P=FALSE;
            NEXT_LATCHES.N=FALSE;
        }
        int sign_mem = (sign_answer & 0x8000) >>15;
        if(sign_mem){
            NEXT_LATCHES.Z=FALSE;
            NEXT_LATCHES.P=FALSE;
            NEXT_LATCHES.N=TRUE;
        }
        if(sign_mem==0){
            NEXT_LATCHES.Z=FALSE;
            NEXT_LATCHES.P=TRUE;
            NEXT_LATCHES.N=FALSE;
        }
    }

}
void branching(int code){
    int off9= (code & 0x01FF);
    int sign = (code & 0x0100)>>8;
    int extended_off9 = sign ? off9 | 0xFE00 : off9 & 0x01FF;
    extended_off9 = extended_off9>>1;
    NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2+extended_off9;

}

void br(int code){
    int nzp = ((code>>9) & 0x000E);
    if(CURRENT_LATCHES.Z==TRUE){
        if(nzp==0x7 || nzp==0x6 || nzp==0x3 || nzp==0x2 || nzp==0x0){
            branching(code);
        }
    }
    if(CURRENT_LATCHES.N==TRUE){
        if(nzp==0x7 || nzp==0x6 || nzp==0x5 || nzp==0x4 || nzp==0x0){
            branching(code);
        }
    }
    if(CURRENT_LATCHES.P==TRUE){
        if(nzp==0x7 || nzp==0x5 || nzp==0x3 || nzp==0x2 || nzp==0x0){
            branching(code);
        }
    }

}
void halt(int code){
    NEXT_LATCHES.PC =0x0000;
}
void jmp(int code){
    int br = ((code & 0x01C0)>>6);
    NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[br];
}
void jsr(int code){
    int sign = (code & 0x0800)>>11;
    int temp = CURRENT_LATCHES.PC+2;
    if(sign){
        int off11= (code & 0x07FF);
        int sign = (code & 0x0700)>>10;
        int extended_off9 = sign ? off11 | 0xF800 : off11 & 0x07FF;
        extended_off9 = extended_off9>>1;
        NEXT_LATCHES.PC= CURRENT_LATCHES.PC+2+extended_off9;
    }
    else{
        int sr1 = ((code & 0x01C0)>>6);
        NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[sr1];
    }
    NEXT_LATCHES.REGS[7] = temp;
}
void ldb(int code){
    int offset6 = code & 0x003F;
    int baseR = (code & 0x01C0)>>6;
    int desR = (code & 0x0E00)>>9;
    int sign = (code & 0x0020)>>5;
    int extended_off6 = sign ? offset6 | 0xFFC0 : offset6 & 0x003F;
    int mem_addr = CURRENT_LATCHES.REGS[baseR]+extended_off6;
    int remainder = mem_addr%2;
    int act_mem = MEMORY[mem_addr/2][remainder];
    if(act_mem==0){
        NEXT_LATCHES.Z=TRUE;
        NEXT_LATCHES.P=FALSE;
        NEXT_LATCHES.N=FALSE;
    }
    int sign_mem=(act_mem & 0x0080)>>7;
    int extend_mem = sign_mem ? act_mem | 0xFF00 : act_mem & 0x00FF;
    NEXT_LATCHES.REGS[desR] = Low16bits(extend_mem);
    if(sign_mem){
        NEXT_LATCHES.Z=FALSE;
        NEXT_LATCHES.P=FALSE;
        NEXT_LATCHES.N=TRUE;
    }
    if(sign_mem==0){
        NEXT_LATCHES.Z=FALSE;
        NEXT_LATCHES.P=TRUE;
        NEXT_LATCHES.N=FALSE;
    }

}
void ldw(int code){
    int offset6 = code & 0x003F;
    int baseR = (code & 0x01C0)>>6;
    int desR = (code & 0x0E00)>>9;
    int sign = (code & 0x0020)>>5;
    int extended_off6 = sign ? offset6 | 0xFFC0 : offset6 & 0x003F;
    extended_off6= extended_off6<<1;
    int mem_addr = CURRENT_LATCHES.REGS[baseR]+extended_off6;
    int first_mem = MEMORY[mem_addr/2][0];
    int second_mem = MEMORY[mem_addr/2][1];
    int act_mem = ((second_mem<<8) & 0xFF00)+(first_mem & 0x00FF);
    if(act_mem==0){
        NEXT_LATCHES.Z=TRUE;
        NEXT_LATCHES.P=FALSE;
        NEXT_LATCHES.N=FALSE;
    }
    int sign_mem = (act_mem & 0x8000) >>15;
    NEXT_LATCHES.REGS[desR] = Low16bits(act_mem);
    if(sign_mem){
        NEXT_LATCHES.Z=FALSE;
        NEXT_LATCHES.P=FALSE;
        NEXT_LATCHES.N=TRUE;
    }
    if(sign_mem==0){
        NEXT_LATCHES.Z=FALSE;
        NEXT_LATCHES.P=TRUE;
        NEXT_LATCHES.N=FALSE;
    }
}
void lea(int code){
    int off9= (code & 0x01FF);
    int sign = (code & 0x0100)>>8;
    int extended_off9 = sign ? off9 | 0xFE00 : off9 & 0x01FF;
    extended_off9= extended_off9<<1;
    int increm_pc = CURRENT_LATCHES.PC+2;
    int desR = (code & 0x0E00)>>9;
    int addy = increm_pc+extended_off9;
    NEXT_LATCHES.REGS[desR] = addy;
}
void lshf(int code){
    int off4 = (code &0x000F);
    int baseR = (code & 0x01C0)>>6;
    int desR = (code & 0x0E00)>>9;
    int value_BaseR = CURRENT_LATCHES.REGS[baseR];
    value_BaseR = value_BaseR <<off4;
    int answer= Low16bits(value_BaseR);
    NEXT_LATCHES.REGS[desR] = answer;
    if(answer==0){
        NEXT_LATCHES.Z=TRUE;
        NEXT_LATCHES.P=FALSE;
        NEXT_LATCHES.N=FALSE;
    }
    int sign_mem = (answer & 0x8000) >>15;
    if(sign_mem){
        NEXT_LATCHES.Z=FALSE;
        NEXT_LATCHES.P=FALSE;
        NEXT_LATCHES.N=TRUE;
    }
    if(sign_mem==0){
        NEXT_LATCHES.Z=FALSE;
        NEXT_LATCHES.P=TRUE;
        NEXT_LATCHES.N=FALSE;
    }
}
void rshfl(int code){
    int off4 = (code &0x000F);
    int baseR = (code & 0x01C0)>>6;
    int desR = (code & 0x0E00)>>9;
    int value_BaseR = CURRENT_LATCHES.REGS[baseR];
    value_BaseR = value_BaseR >>off4;
    int answer= Low16bits(value_BaseR);
    NEXT_LATCHES.REGS[desR] = answer;
    if(answer==0){
        NEXT_LATCHES.Z=TRUE;
        NEXT_LATCHES.P=FALSE;
        NEXT_LATCHES.N=FALSE;
    }
    int sign_mem = (answer & 0x8000) >>15;
    if(sign_mem){
        NEXT_LATCHES.Z=FALSE;
        NEXT_LATCHES.P=FALSE;
        NEXT_LATCHES.N=TRUE;
    }
    if(sign_mem==0){
        NEXT_LATCHES.Z=FALSE;
        NEXT_LATCHES.P=TRUE;
        NEXT_LATCHES.N=FALSE;
    }
}
void rshfa(int code){
    int off4 = (code &0x000F);
    int baseR = (code & 0x01C0)>>6;
    int desR = (code & 0x0E00)>>9;
    int value_BaseR = CURRENT_LATCHES.REGS[baseR];
    int sign = (value_BaseR & 0x8000) >>15;
    int extended_Value_BaseR = sign ? value_BaseR | 0xFFFF0000 : value_BaseR & 0x0000FFFF;
    value_BaseR = extended_Value_BaseR >>off4;
    int answer= Low16bits(value_BaseR);
    NEXT_LATCHES.REGS[desR] = answer;
    if(answer==0){
        NEXT_LATCHES.Z=TRUE;
        NEXT_LATCHES.P=FALSE;
        NEXT_LATCHES.N=FALSE;
    }
    int sign_mem = (answer & 0x8000) >>15;
    if(sign_mem){
        NEXT_LATCHES.Z=FALSE;
        NEXT_LATCHES.P=FALSE;
        NEXT_LATCHES.N=TRUE;
    }
    if(sign_mem==0){
        NEXT_LATCHES.Z=FALSE;
        NEXT_LATCHES.P=TRUE;
        NEXT_LATCHES.N=FALSE;
    }

}
void stb(int code){
    int offset6 = code & 0x003F;
    int baseR = (code & 0x01C0)>>6;
    int desR = (code & 0x0E00)>>9;
    int sign = (code & 0x0020)>>5;
    int extended_off6 = sign ? offset6 | 0xFFC0 : offset6 & 0x003F;
    int address = extended_off6+CURRENT_LATCHES.REGS[baseR];
    int address_offset = address%2;
    address = address>>1;
    int answer = MEMORY[address][address_offset];
    NEXT_LATCHES.REGS[desR] = Low16bits(answer);

}
void stw(int code){
    int offset6 = code & 0x003F;
    int baseR = (code & 0x01C0)>>6;
    int desR = (code & 0x0E00)>>9;
    int sign = (code & 0x0020)>>5;
    int extended_off6 = sign ? offset6 | 0xFFC0 : offset6 & 0x003F;
    extended_off6=extended_off6<<1;
    int address = extended_off6+CURRENT_LATCHES.REGS[baseR];
    address = address>>1;
    int answer_part1 = MEMORY[address][0];
    int answer_part2 = MEMORY[address][1];
    int act_mem = ((answer_part2<<8) & 0xFF00)+(answer_part1 & 0x00FF);
    NEXT_LATCHES.REGS[desR] = Low16bits(act_mem);
}
void trap(int code){
    NEXT_LATCHES.PC =0x0000;
}
void xor(int code){
    int flag= ((code & 0x0020)>>5);
    int dr = ((code & 0x0E00)>>9);
    int sr1 = ((code & 0x01C0)>>6);
    if(flag==0){
        int sr2 =(code & 0x0007);
        int answer  = CURRENT_LATCHES.REGS[sr1]^CURRENT_LATCHES.REGS[sr2];
        int sign_answer= Low16bits(answer);
        NEXT_LATCHES.REGS[dr]= sign_answer;
        if(sign_answer==0){
            NEXT_LATCHES.Z=TRUE;
            NEXT_LATCHES.P=FALSE;
            NEXT_LATCHES.N=FALSE;
        }
        int sign_mem = (sign_answer & 0x8000) >>15;
        if(sign_mem){
            NEXT_LATCHES.Z=FALSE;
            NEXT_LATCHES.P=FALSE;
            NEXT_LATCHES.N=TRUE;
        }
        if(sign_mem==0){
            NEXT_LATCHES.Z=FALSE;
            NEXT_LATCHES.P=TRUE;
            NEXT_LATCHES.N=FALSE;
        }
    }
    else{
        int imm5= (code & 0x001F);
        int sign = (code & 0x0010)>>4;
        int extend_imm = sign ? imm5 | 0xFFE0 : imm5 & 0x001F;
        int answer= CURRENT_LATCHES.REGS[sr1] ^extend_imm;
        int sign_answer= Low16bits(answer);
        NEXT_LATCHES.REGS[dr]= sign_answer;
        if(sign_answer==0){
            NEXT_LATCHES.Z=TRUE;
            NEXT_LATCHES.P=FALSE;
            NEXT_LATCHES.N=FALSE;
        }
        int sign_mem = (sign_answer & 0x8000) >>15;
        if(sign_mem){
            NEXT_LATCHES.Z=FALSE;
            NEXT_LATCHES.P=FALSE;
            NEXT_LATCHES.N=TRUE;
        }
        if(sign_mem==0){
            NEXT_LATCHES.Z=FALSE;
            NEXT_LATCHES.P=TRUE;
            NEXT_LATCHES.N=FALSE;
        }
    }
}


typedef void simul_instruct(int code);


void process_instruction(){
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */
  // once you get the memory
    // you first convert the hex number to a binary number
    // then based on the first four digit input you choose which instruction you need. - you do that with a switch case
// you get hte address - move it 8 bits to th
    int current_PC = CURRENT_LATCHES.PC;
    current_PC=current_PC>>1;
    int first_mem = MEMORY[current_PC][0];
    int second_mem = MEMORY[current_PC][1];
    int code = ((second_mem<<8) & 0xFF00)+(first_mem & 0x00FF);
    //printf("%X\n",code);
    int opcode = ((code>>12) & 0x000F);
    int shift = ((code>>4) & 0x0003);
    if(code==0x0000){
        //NOP instruction
        return;
    }
    if(code==0xF025){
        halt(code);
        return;
    }
    switch (opcode) {
        case 0x1:
            // this is the add function
            add(code);
            NEXT_LATCHES.PC = CURRENT_LATCHES.PC+2;
            break;
        case 0x5:
            //this is the and function
            and(code);
            NEXT_LATCHES.PC = CURRENT_LATCHES.PC+2;
            break;
        case 0x0:
            //this is the branch functino
            br(code);
            break;

        case 0xC:
            //this is the jump and ret function
            //first check whether its jump or ret
            jmp(code);
            break;
        case 0x4:
            //this is the jsr and jsrr function
            jsr(code);
            break;
        case 0x2:
            //this is the ldb function
            ldb(code);
            NEXT_LATCHES.PC = CURRENT_LATCHES.PC+2;
            break;
        case 0x6:
            // this is the ldw function
            ldw(code);
            NEXT_LATCHES.PC = CURRENT_LATCHES.PC+2;
            break;
        case 0xE:
            // this is the lea function
            lea(code);
            NEXT_LATCHES.PC = CURRENT_LATCHES.PC+2;
            break;
        case 0xD:
            // this is the lsfh rshfl rshfa function
            if(shift==0x0){
                lshf(code);
            }
            if(shift==0x1){
                rshfl(code);
            }
            if(shift==0x3){
                rshfa(code);
            }
            NEXT_LATCHES.PC = CURRENT_LATCHES.PC+2;
            break;
        case 0x3:
            //this is the stb function
            stb(code);
            NEXT_LATCHES.PC = CURRENT_LATCHES.PC+2;
            break;
        case 0x7:
            // this is the stw function
            stw(code);
            NEXT_LATCHES.PC = CURRENT_LATCHES.PC+2;
            break;
        case 0xF:
            // this is the trap function
            trap(code);
        case 0x9:
            // this is the xor and not function
            xor(code);
            NEXT_LATCHES.PC = CURRENT_LATCHES.PC+2;
            break;
        default:
            exit(EXIT_FAILURE);
    }









//    simul_instruct *my_array[16] = {
//            add,
//            and,
//            br,
//            brn,
//            brp,
//            brnp,
//            brz,
//            brnz,
//            brzp,
//            brnzp,
//            halt,
//            jmp,
//            jsr,
//            jsrr,
//            ldb,
//            ldw,
//            lea,
//            nop,
//            not,
//            ret,
//            lshf,
//            rshfl,
//            rshfa,
//            stb,
//            stw,
//            trap,
//            xor,
//
//
//    };
//        my_array[]
}