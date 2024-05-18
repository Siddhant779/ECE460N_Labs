#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#define SYM_NAME_CAP 24
#define SYMTBL_CAP   256
#define LINE_CAP     256

enum Opcode {
    // enumeration for all the types of opcodes possible
    OPC_NONE,
    OPC_END,
    OPC_ORIG,
    OPC_HALT,
    OPC_NOP,
    OPC_FILL,
    OPC_ADD,
    OPC_AND,
    OPC_JMP,
    OPC_JSR,
    OPC_JSRR,
    OPC_LDB,
    OPC_LDW,
    OPC_LEA,
    OPC_NOT,
    OPC_RET,
    OPC_RTI,
    OPC_LSHF,
    OPC_RSHFL,
    OPC_RSHFA,
    OPC_STB,
    OPC_STW,
    OPC_TRAP,
    OPC_XOR,
    OPC_BRN,
    OPC_BRP,
    OPC_BRNP,
    OPC_BR,
    OPC_BRZ,
    OPC_BRNZ,
    OPC_BRZP,
    OPC_BRNZP,

    // TODO finish
};

struct Files {
    // points to the input file and the output file
    FILE *src, *out;
};

struct Symbol {
    int value; // has the address of the symbol
    char name[SYM_NAME_CAP];
};

struct SymbolTable {
    int len; // the length of the symbolTable - keeps track of the size of the symbols in the program
    struct Symbol symbols[SYMTBL_CAP]; //contains the array of symbols in the program
};

struct Line {
    char *label, *args[3]; // how the line is organized - has a label, instruction/opcode, and the arguments that are needed for that instruction to happen
    enum Opcode opcode; // what opcode is being utilized in this line
};

struct Files open_files(char **args, int args_len) {
    // this is what is responsible for populating the Files struct - this puts everything in the file struct
    // first argument is the program name
    // second argument is the input file
    // third argument is the output file
    struct Files result;
    if (args_len != 3) {
        exit(EXIT_FAILURE);
    }
    result.src = fopen(args[1], "r");
    if (!result.src) {
        printf("Error: Cannot open file %s\n", args[1]);
        exit(EXIT_FAILURE);
    }
    result.out = fopen(args[2], "w");
    if (!result.out) {
        printf("Error: Cannot open file %s\n", args[2]);
        exit(EXIT_FAILURE);
    }
    if (!result.src || !result.out) exit(EXIT_FAILURE);
    return result;

}

enum Opcode isOpcode(char *lPtr) {
    if(strcmp(lPtr,".orig")==0){
        return OPC_ORIG;
    }
    if(strcmp(lPtr,".end")==0){
        return OPC_END;
    }
    if (strcmp(lPtr, "add") == 0) {
        return OPC_ADD;
    }
    if (strcmp(lPtr, "and") == 0) {
        return OPC_AND;
    }
    if(strcmp(lPtr,"br")==0){
        return OPC_BR;
    }
    if (strcmp(lPtr, "brn") == 0) {
        return OPC_BRN;
    }
    if (strcmp(lPtr, "brp") == 0) {
        return OPC_BRP;
    }
    if (strcmp(lPtr, "brnp") == 0) {
        return OPC_BRNP;
    }
    if (strcmp(lPtr, "brz") == 0) {
        return OPC_BRZ;
    }
    if (strcmp(lPtr, "brnz") == 0) {
        return OPC_BRNZ;
    }
    if (strcmp(lPtr, "brzp") == 0) {
        return OPC_BRZP;
    }
    if (strcmp(lPtr, "brnzp") == 0) {
        return OPC_BRNZP;
    }
    if (strcmp(lPtr, "halt") == 0) {
        return OPC_HALT;
    }
    if (strcmp(lPtr, "jmp") == 0) {
        return OPC_JMP;
    }
    if (strcmp(lPtr, "jsr") == 0) {
        return OPC_JSR;
    }
    if (strcmp(lPtr, "jsrr") == 0) {
        return OPC_JSRR;
    }
    if (strcmp(lPtr, "ldb") == 0) {
        return OPC_LDB;
    }
    if (strcmp(lPtr, "ldw") == 0) {
        return OPC_LDW;
    }
    if (strcmp(lPtr, "lea") == 0) {
        return OPC_LEA;
    }
    if (strcmp(lPtr, "nop") == 0) {
        return OPC_NOP;
    }
    if (strcmp(lPtr, "not") == 0) {
        return OPC_NOT;
    }
    if (strcmp(lPtr, "ret") == 0) {
        return OPC_RET;
    }
    if (strcmp(lPtr, "lshf") == 0) {
        return OPC_LSHF;
    }
    if (strcmp(lPtr, "rshfl") == 0) {
        return OPC_RSHFL;
    }
    if (strcmp(lPtr, "rshfa") == 0) {
        return OPC_RSHFA;
    }
    if (strcmp(lPtr, "rti") == 0) {
        return OPC_RTI;
    }
    if (strcmp(lPtr, "stb") == 0) {
        return OPC_STB;
    }
    if (strcmp(lPtr, "stw") == 0) {
        return OPC_STW;
    }
    if (strcmp(lPtr, "trap") == 0) {
        return OPC_TRAP;
    }
    if (strcmp(lPtr, "xor") == 0) {
        return OPC_XOR;
    }
    if(strcmp(lPtr,".fill")==0){
        return OPC_FILL;
    }
    else{
        return OPC_NONE;
    }
}

struct Line parse_next_line(char *line_str, FILE *file) {
    // needs to return the line struct - this is waht will be seperating the lines together
    struct Line finLine = {.opcode = OPC_NONE};
    fgets(line_str, LINE_CAP, file); // once it encounters the new line it stops right?
    char *cmt = strchr(line_str, ';');
    if (cmt) *cmt = '\0';
    for (int i = 0; line_str[i]; i++) {
        line_str[i] = tolower(line_str[i]);
    }
    char *lPtr = strtok(line_str, "\n\t ");
    if(!lPtr){
        return finLine;
    }
    if (isOpcode(lPtr) == OPC_NONE && lPtr[0] != '.') /* found a label */
    {
        finLine.label = lPtr;
        if ((lPtr = strtok(NULL, "\t\n ,")) == NULL) {
            return finLine;
        }
    }
    enum Opcode opc = isOpcode(lPtr);
    finLine.opcode=opc;
    if ((lPtr = strtok(NULL, "\t\n ,")) == NULL) {
        return finLine;
    }
    finLine.args[0] = lPtr;
    if ((lPtr = strtok(NULL, "\t\n ,")) == NULL) {
        return finLine;
    }
    finLine.args[1] = lPtr;
    if ((lPtr = strtok(NULL, "\t\n ,")) == NULL) {
        return finLine;
    }
    finLine.args[2] = lPtr;
    return finLine;

}

void add_symbol(struct SymbolTable *table, char *name, int value) {
    // this is what will be adding labels to the symbol table
    //didnt have this as a pointer before
    struct Symbol *symbol = &table->symbols[table->len];
    symbol->value=value;
    strcpy(symbol->name,name);
    table->len ++;
}

int convert_num(char *arg) {
    // it will either come with the x in the front or it might come with the # in the front need to check for that
    //first for doing number
    if (!arg) return 0;

    switch (arg[0]) {
        case '#':
            return strtol(&arg[1], NULL, 10);
        case 'x':
            return strtol(&arg[1], NULL, 16);
        default:
            return 0;
    }
}
int lookup_symbol(struct SymbolTable *table, char *name) {
    for(int i=0;i<table->len;i++){
        if(strcmp(table->symbols[i].name,name)==0){
            return table->symbols[i].value;
        }
    }
    return -1;
}
int registerCalc(char *reg){
    if(strcmp("r7",reg)==0){
        return 0x7;
    }
    if(strcmp("r6",reg)==0){
        return 0x6;
    }
    if(strcmp("r5",reg)==0){
        return 0x5;
    }
    if(strcmp("r4",reg)==0){
        return 0x4;
    }
    if(strcmp("r3",reg)==0){
        return 0x3;
    }
    if(strcmp("r2",reg)==0){
        return 0x2;
    }
    if(strcmp("r1",reg)==0){
        return 0x1;
    }
    if(strcmp("r0",reg)==0){
        return 0x0;
    }
    else{
        return -1;
    }
}
int is_reg(char *arg) {
    if (!arg) return 0;
    return arg[0] == 'r' && isdigit(arg[1]) && !arg[2];
}
int assembly_concat_rr6(int op,int arg1, int arg2, int arg3){
    //remember to check if arg3 is a number first and use the convert number if it is
    //add and xor ldb ldw
    return op << 12 | (arg1 & 0x7) << 9 | (arg2 & 0x7) << 6 | (arg3 & 0x3F);
}
int assembly_concat_r9(int op,int arg1,int arg2){
    return op << 12 | (arg1 & 0x7) << 9 | (arg2 >> 1 & 0x1FF);
}
int assembly_concat_r11(int op,int offset){
    return op << 12 | 1 << 11 | (offset >> 1 & 0x7FF);
}
int assemble_line(struct Line line, struct SymbolTable *table, int address) {
    // this is the meat of the code - actaully outputting - need to have multiple case in this one
    // need to group specific assembly instructions so that its easier to output a line
    int values[3] = {0};
    for (int i = 0; i < 3 && line.args[i]; i++) {
        if(lookup_symbol(table,line.args[i])!=-1){
            values[i]= lookup_symbol(table,line.args[i]);
        }
        else if(registerCalc(line.args[i])!=-1){
            values[i]= registerCalc(line.args[i]);
        }
        else{
            values[i] = convert_num(line.args[i]);
        }
    }
    int yes_reg = !is_reg(line.args[2]) << 5;
    switch(line.opcode){
        case OPC_FILL:
            return values[0] & 0xFFFF;
        case OPC_ADD:
            return assembly_concat_rr6(0x1,values[0],values[1],yes_reg | values[2]);
        case OPC_AND:
            return assembly_concat_rr6(0x5,values[0],values[1],yes_reg | values[2]);
        case OPC_BR:
            return assembly_concat_r9(0x0,7,values[0]-address);
        case OPC_BRN:
            return assembly_concat_r9(0x0,4,values[0]-address);
        case OPC_BRP:
            return assembly_concat_r9(0x0,1,values[0]-address);
        case OPC_BRNP:
            return assembly_concat_r9(0x0,5,values[0]-address);
        case OPC_BRZ:
            return assembly_concat_r9(0x0,2,values[0]-address);
        case OPC_BRNZ:
            return assembly_concat_r9(0x0,6,values[0]-address);
        case OPC_BRZP:
            return assembly_concat_r9(0x0,3,values[0]-address);
        case OPC_BRNZP:
            return assembly_concat_r9(0x0,7,values[0]-address);
        case OPC_HALT:
            return 0xF025;
        case OPC_JMP:
            return assembly_concat_rr6(0xC,0,values[0],0);
        case OPC_JSR:
            return assembly_concat_r11(0x4,values[0]-address);
        case OPC_JSRR:
            return assembly_concat_rr6(0x4,0,values[0],0);
        case OPC_LDB:
            return assembly_concat_rr6(0x2,values[0],values[1],values[2]);
        case OPC_LDW:
            return assembly_concat_rr6(0x6,values[0],values[1],values[2]);
        case OPC_LEA:
            return assembly_concat_r9(0xE,values[0],values[1]-address);
            //return 0x1111;
        case OPC_NOP:
            return 0x0000;
        case OPC_NOT:
            return assembly_concat_rr6(0x9, values[0], values[1], 0x3F);
        case OPC_RET:
            return assembly_concat_rr6(0xC, 0, 7, 0);
        case OPC_LSHF:
            return assembly_concat_rr6(0xD, values[0], values[1], 0x00 | values[2]);
        case OPC_RSHFL:
            return assembly_concat_rr6(0xD, values[0], values[1], 0x10 | values[2]);
        case OPC_RSHFA:
            return assembly_concat_rr6(0xD, values[0], values[1], 0x30 | values[2]);
        case OPC_RTI:
            return 0x8000;
        case OPC_STB:
            return assembly_concat_rr6(0x3, values[0], values[1], values[2]);
        case OPC_STW:
            return assembly_concat_rr6(0x7, values[0], values[1], values[2]);
        case OPC_TRAP:
            return 0xF000 | values[0];
        case OPC_XOR:
            return assembly_concat_rr6(0x9, values[0], values[1], yes_reg | values[2]);
        default:
             exit(EXIT_FAILURE);
    }
}

void out_code(int code, FILE *file) {
    fprintf(file, "0x%.4X\n", code);
}

void gen_symbols(struct SymbolTable *table, struct Files files) {
    int address = 0;
    while (1) {
        char line_str[LINE_CAP]; // how big the entire c file is
        struct Line line = parse_next_line(line_str, files.src); // this is what will be populating the Line struct
        printf("got the line");
        if (line.label != NULL) {
            add_symbol(table, line.label, address);
        }
        switch (line.opcode) {
            case OPC_NONE:
                break;
            case OPC_END:
                return;
            case OPC_ORIG:
                address = convert_num(line.args[0]);
                break;
            default:
                address += 2;
                break;
        }
    }
}

void gen_code(struct SymbolTable *table, struct Files files) {
    int address = 0;
    while (1) {
        char line_str[LINE_CAP];
        struct Line line = parse_next_line(line_str, files.src);
//        int code=3131;
//        fprintf(files.out, "0x%.4X\n", code);
        switch (line.opcode) {
            case OPC_NONE:
                break;
            case OPC_END:
                return;
            case OPC_ORIG:
                address = convert_num(line.args[0]);
                out_code(address, files.out);
                break;
            default:
                address += 2; // moved it to the front since it increments the PC before going to the next instruction
                out_code(assemble_line(line, table, address), files.out);
                break;
        }
    }
}

int main(int args_len, char **args) {
    struct Files files = open_files(args, args_len); // this will first populate the struct file
    struct SymbolTable table = {.len = 0}; // this part not sure
    gen_symbols(&table, files); // populates the symbol table - this for the first pass through
    rewind(files.src); // goes back to the start of the file
    gen_code(&table, files); // this is the second pass through which converts the c code to instruction code
    fclose(files.src);
    fclose(files.out);
    exit(EXIT_SUCCESS);
}




