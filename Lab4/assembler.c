/*
	Name 1: Ben Endara 
	Name 2: Siddhant Pandit 
	UTEID 1: bbe276
	UTEID 2: shp695
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define SYM_NAME_CAP 24
#define SYMTBL_CAP   256
#define LINE_CAP     256

enum Opcode {
	OPC_NONE, OPC_ORIG,  OPC_END,   OPC_FILL,
	OPC_ADD,  OPC_AND,   OPC_BR,    OPC_BRN,
	OPC_BRZ,  OPC_BRP,   OPC_BRNZ,  OPC_BRNP,
	OPC_BRZP, OPC_BRNZP, OPC_HALT,  OPC_JMP,
	OPC_JSR,  OPC_JSRR,  OPC_LDB,   OPC_LDW,
	OPC_LEA,  OPC_NOP,   OPC_NOT,   OPC_RET,
	OPC_LSHF, OPC_RSHFL, OPC_RSHFA, OPC_RTI,
	OPC_STB,  OPC_STW,   OPC_TRAP,  OPC_XOR,
};

struct Files {
	FILE *src, *out;
};

struct Symbol {
	int value;
	char name[SYM_NAME_CAP];
};

struct SymbolTable {
	int len;
	struct Symbol symbols[SYMTBL_CAP];
};

struct Line {
	char *label, *args[3];
	enum Opcode opcode;
};

static struct Files open_files(char **args, int args_len);
static void gen_symbols(struct SymbolTable *table, struct Files files);
static void gen_code(struct SymbolTable *table, struct Files files);
static void out_code(int code, FILE *file);
static struct Line parse_next_line(char *line_str, FILE *file);
static enum Opcode convert_instr(char *instr);
static void next_line_str(char *line_str, FILE *file);
static void add_symbol(struct SymbolTable *table, char *name, int value);
static int lookup_symbol(struct SymbolTable *table, char *name);
static int assemble_line(struct Line line, struct SymbolTable *table, int address);
static int fmt_rrs(int op, int reg1, int reg2, int spec);
static int fmt_ro9(int op, int reg, int off);
static int fmt_o11(int op, int off);
static int is_symbol(char *arg);
static int is_reg(char *arg);
static int convert_arg(char *arg);

int main(int args_len, char **args) {
	struct Files files = open_files(args, args_len);

	// First pass
	struct SymbolTable table = {.len = 0};
	gen_symbols(&table, files);

	// Second pass
	rewind(files.src);
	gen_code(&table, files);

	exit(EXIT_SUCCESS);
}

struct Files open_files(char **args, int args_len) {
	if (args_len != 3) exit(EXIT_FAILURE);
	struct Files files = {
		.src = fopen(args[1], "r"),
		.out = fopen(args[2], "w"),
	};
	if (!files.src || !files.out) exit(EXIT_FAILURE);
	return files;
}

void gen_symbols(struct SymbolTable *table, struct Files files) {
	int address = 0;
	while (1) {
		char line_str[LINE_CAP];
		struct Line line = parse_next_line(line_str, files.src);
		
		// Add symbol if the line has a label
		if (line.label) add_symbol(table, line.label, address);
		
		switch (line.opcode) {
			case OPC_NONE: break;
			case OPC_END:  return;
			case OPC_ORIG:
				address = convert_arg(line.args[0]);
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
		
		switch (line.opcode) {
			case OPC_NONE: break;
			case OPC_END:  return;
			case OPC_ORIG:
				address = convert_arg(line.args[0]);
				out_code(address, files.out);
				break;
			default:
				out_code(assemble_line(line, table, address), files.out);
				address += 2;
				break;
		}
	}
}

void out_code(int code, FILE *file) {
	fprintf(file, "0x%.4X\n", code);
}

struct Line parse_next_line(char *line_str, FILE *file) {
	next_line_str(line_str, file);
	struct Line line = {.opcode = OPC_NONE};
	
	// Parse label and instruction
	char *tok1 = strtok(line_str, "\n\t ");
	if (!tok1) {
		// Empty line
		return line;
	}
	enum Opcode opc = convert_instr(tok1);
	if (opc != OPC_NONE) {
		// Instruction only
		line.opcode = opc;
	}
	else {
		char *tok2 = strtok(NULL, "\n\t ");
		if (!tok2) {
			// Label only
			line.label = tok1;
			return line;
		}
		opc = convert_instr(tok2);
		if (opc == OPC_NONE) exit(EXIT_FAILURE);
		// Both label and instruction
		line.label  = tok1;
		line.opcode = opc;
	}

	// Parse instruction arguments
	for (int i = 0; i < 3; ++i) {
		line.args[i] = strtok(NULL, "\n\t ,");
		if (!line.args[i]) break;
	}

	return line;
}

enum Opcode convert_instr(char *instr) {
	static struct {
		enum Opcode opcode;
		char *instr;
	} opcode_tbl[] = {
		{OPC_ORIG,  ".orig"}, {OPC_END,   ".end"},  
		{OPC_FILL,  ".fill"}, {OPC_ADD,   "add"},  
		{OPC_AND,   "and"},   {OPC_BR,    "br"},   
		{OPC_BRN,   "brn"},   {OPC_BRZ,   "brz"},  
		{OPC_BRP,   "brp"},   {OPC_BRNZ,  "brnz"}, 
		{OPC_BRNP,  "brnp"},  {OPC_BRZP,  "brzp"}, 
		{OPC_BRNZP, "brnzp"}, {OPC_HALT,  "halt"}, 
		{OPC_JMP,   "jmp"},   {OPC_JSR,   "jsr"},  
		{OPC_JSRR,  "jsrr"},  {OPC_LDB,   "ldb"},  
		{OPC_LDW,   "ldw"},   {OPC_LEA,   "lea"},  
		{OPC_NOP,   "nop"},   {OPC_NOT,   "not"},  
		{OPC_RET,   "ret"},   {OPC_LSHF,  "lshf"}, 
		{OPC_RSHFL, "rshfl"}, {OPC_RSHFA, "rshfa"},
		{OPC_RTI,   "rti"},   {OPC_STB,   "stb"},  
		{OPC_STW,   "stw"},   {OPC_TRAP,  "trap"}, 
		{OPC_XOR,   "xor"},
	};

	// Search table for instr
	for (int i = 0; i < sizeof opcode_tbl / sizeof *opcode_tbl; ++i) {
		if (strcmp(instr, opcode_tbl[i].instr) == 0) return opcode_tbl[i].opcode;
	}
	return OPC_NONE;
}

// Fills line_str with next line from file
void next_line_str(char *line_str, FILE *file) {
	if (!fgets(line_str, LINE_CAP, file)) exit(EXIT_FAILURE);

	// Trim comment from line
	char *cmt = strchr(line_str, ';');
	if (cmt) *cmt = '\0';

	// Make line lowercase
	for (int i = 0; line_str[i]; ++i) {
		line_str[i] = tolower(line_str[i]);
	}
}

void add_symbol(struct SymbolTable *table, char *name, int value) {
	// Ensure space in table
	if (table->len >= SYMTBL_CAP) exit(EXIT_FAILURE);

	// Ensure symbol is not too long
	if (strlen(name) >= SYM_NAME_CAP) exit(EXIT_FAILURE);
	
	// Instantiate new symbol
	struct Symbol *sym = &table->symbols[table->len++];
	strcpy(sym->name, name);
	sym->value = value;
}

int lookup_symbol(struct SymbolTable *table, char *name) {
	// Search table for name
	for (int i = 0; i < table->len; ++i) {
		if (strcmp(name, table->symbols[i].name) == 0) return table->symbols[i].value;
	}
	exit(EXIT_FAILURE);
}

// Returns machine code for a line
int assemble_line(struct Line line, struct SymbolTable *table, int address) {
	// Convert arguments to values
	int vals[3] = {0};
	for (int i = 0; i < 3 && line.args[i]; ++i) {
		vals[i] = is_symbol(line.args[i]) ?
			lookup_symbol(table, line.args[i]) :
			convert_arg(line.args[i]);
	}

	// Calculate arithmetic immediate flag
	int imm_flag = !is_reg(line.args[2]) << 5;

	// Construct machine code
	switch (line.opcode) {
		case OPC_FILL:  return vals[0] & 0xFFFF;
		case OPC_ADD:   return fmt_rrs(0x1, vals[0], vals[1], imm_flag | vals[2]);
		case OPC_AND:   return fmt_rrs(0x5, vals[0], vals[1], imm_flag | vals[2]);
		case OPC_BR:    return fmt_ro9(0x0, 7, vals[0] - address - 2);
		case OPC_BRN:   return fmt_ro9(0x0, 4, vals[0] - address - 2);
		case OPC_BRZ:   return fmt_ro9(0x0, 2, vals[0] - address - 2);
		case OPC_BRP:   return fmt_ro9(0x0, 1, vals[0] - address - 2);
		case OPC_BRNZ:  return fmt_ro9(0x0, 6, vals[0] - address - 2);
		case OPC_BRNP:  return fmt_ro9(0x0, 5, vals[0] - address - 2);
		case OPC_BRZP:  return fmt_ro9(0x0, 3, vals[0] - address - 2);
		case OPC_BRNZP: return fmt_ro9(0x0, 7, vals[0] - address - 2);
		case OPC_HALT:  return 0xF025;
		case OPC_JMP:   return fmt_rrs(0xC, 0, vals[0], 0);
		case OPC_JSR:   return fmt_o11(0x4, vals[0] - address - 2);
		case OPC_JSRR:  return fmt_rrs(0x4, 0, vals[0], 0);
		case OPC_LDB:   return fmt_rrs(0x2, vals[0], vals[1], vals[2]);
		case OPC_LDW:   return fmt_rrs(0x6, vals[0], vals[1], vals[2]);
		case OPC_LEA:   return fmt_ro9(0xE, vals[0], vals[1] - address - 2);
		case OPC_NOP:   return 0x0000;
		case OPC_NOT:   return fmt_rrs(0x9, vals[0], vals[1], 0x3F);
		case OPC_RET:   return fmt_rrs(0xC, 0, 7, 0);
		case OPC_LSHF:  return fmt_rrs(0xD, vals[0], vals[1], 0x00 | vals[2]);
		case OPC_RSHFL: return fmt_rrs(0xD, vals[0], vals[1], 0x10 | vals[2]);
		case OPC_RSHFA: return fmt_rrs(0xD, vals[0], vals[1], 0x30 | vals[2]);
		case OPC_RTI:   return 0x8000;
		case OPC_STB:   return fmt_rrs(0x3, vals[0], vals[1], vals[2]);
		case OPC_STW:   return fmt_rrs(0x7, vals[0], vals[1], vals[2]);
		case OPC_TRAP:  return 0xF000 | vals[0] & 0xFF;
		case OPC_XOR:   return fmt_rrs(0x9, vals[0], vals[1], imm_flag | vals[2]);
		default:        exit(EXIT_FAILURE);
	}
}

// Register, register, operation specific
int fmt_rrs(int op, int reg1, int reg2, int spec) {
	return op << 12 | (reg1 & 0x7) << 9 | (reg2 & 0x7) << 6 | (spec & 0x3F);
}

// Register, 9-bit offset
int fmt_ro9(int op, int reg, int off) {
	return op << 12 | (reg & 0x7) << 9 | (off >> 1 & 0x1FF);
}

// 11-bit offset
int fmt_o11(int op, int off) {
	return op << 12 | 1 << 11 | (off >> 1 & 0x7FF);
}

int is_symbol(char *arg) {
	if (!arg) return 0;

	// Ensure arg is not number or register
	if (!isalpha(arg[0]) || arg[0] == 'x' || is_reg(arg)) return 0;
	
	// Ensure arg is alphanumeric
	for (int i = 0; arg[i]; ++i) {
		if (!isalnum(arg[i])) return 0;
	}

	return 1;
}

int is_reg(char *arg) {
	if (!arg) return 0;
	return arg[0] == 'r' && isdigit(arg[1]) && !arg[2];
}

// Converts decimal numbers, hex numbers, and registers
int convert_arg(char *arg) {
	if (!arg) return 0;
	
	switch (arg[0]) {
		case '#': return strtol(&arg[1], NULL, 10);
		case 'x': return strtol(&arg[1], NULL, 16);
		case 'r': return isdigit(arg[1]) ? arg[1] - '0' : 0;
		default:  return 0;
	}
}