#ifndef CPU_ACTIVE
#define CPU_ACTIVE

#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include "stack.hpp"

#define NO_OFFSET 0
#define STRINGS_EQUAL 0
#define ERROR -1
#define SUCCESS 0
#define FRIDAY 5

constexpr int SIGNATURE = 'QO';

constexpr int NUM_OF_REGS = 4;
constexpr int RAM_SIZE = 1048576;

constexpr int MAX_CMD_NAME_BYTE_SIZE = 8;
constexpr int MIN_CMD_NAME_BYTE_SIZE = 2;
constexpr int MAX_MARK_NAME_BYTE_SIZE = MAX_CMD_NAME_BYTE_SIZE - sizeof (char);
constexpr int MIN_MARK_NAME_BYTE_SIZE = MIN_CMD_NAME_BYTE_SIZE - sizeof (char);
constexpr int MAX_LISTING_LINE_SIZE = 49;
constexpr int DEFAULT_MARK_TABLE_ELEM_SIZE = 8;

struct cpu_t {

    unsigned char *code;
    int arch;
    int *ram;
    int reg [NUM_OF_REGS];
    stack_t stack;
    addr_stack_t addr_stack;
    size_t ip;
    size_t code_size;
};

struct code_info_t {

    int sig;
    int arch;
};

struct mark_t {

    char name [MAX_MARK_NAME_BYTE_SIZE + sizeof (char)];
    size_t idx;
};

constexpr char REG_BIT_MASK = 128;
constexpr char IMM_BIT_MASK = 64;
constexpr char RAM_BIT_MASK = 32;
constexpr char ONLY_CMD_TYPE_MASK = 31;

enum CMD_NAME_CNSTS {HLT = 0, PUSH = 1, POP = 2, ADD = 3, SUB = 4, MUL = 5, DIV = 6, IN = 7, OUT = 8, STDMP = 9, STVRF = 10, RGDMP = 11,            \
                     JMP = 12, JA = 13, JAE = 14, JB = 15, JBE = 16, JE = 17, JNE = 18, JF = 19, CALL = 20, RET = 21, ASTDMP = 22, ASTVRF = 23};

const char *CMD_NAMES [] = {"hlt" /* 0 */, "push" /* 1 */, "pop" /* 2 */, "add" /* 3 */, "sub" /* 4 */, "mul" /* 5 */,              \
                            "div" /* 6 */, "in" /* 7 */, "out" /* 8 */, "stdmp" /* 9 */, "stvrf" /* 10 */, "rgdmp" /* 11 */,        \
                            "jmp" /* 12 */, "ja" /* 13 */, "jae" /* 14 */, "jb" /* 15 */, "jbe" /* 16 */, "je" /* 17 */,            \
                            "jne" /* 18 */, "jf" /* 19 */, "call" /* 20 */, "ret" /* 21 */, "astdmp" /* 22 */, "astvrf" /* 23 */};  \

constexpr unsigned char NUM_OF_CMD_TYPES = 24;

/* cpu functions */
void cpu_ctor (cpu_t *cpu);
void cpu_dtor (cpu_t *cpu);
void load_code (FILE *code_file, cpu_t *cpu);
int execute_code (cpu_t *cpu);

/* asm functions */
int assemble_prog (char **cmd_index_tbl, int num_of_cmds, unsigned char *code_buffer, size_t *assembled_code_size, mark_t *mark_tbl, int num_of_marks);
int preassemble_prog (char **cmd_index_tbl, int num_of_cmds, mark_t **mark_tbl, int *num_of_marks);
void set_code_info (unsigned char **code_buffer);
char *store_cmds (FILE *prog_file);
void upload_code (unsigned char *code_buffer, FILE *code_file, size_t assembled_code_size);
char **index_cmds (char *cmd_buffer, int num_of_cmds);
void clean_asm_memory (char **cmd_index_tbl, char *cmd_buffer, unsigned char *code_buffer, mark_t *mark_tbl);
int get_num_of_cmds (char *cmd_buffer);
size_t estimate_code_size (char *cmd_index_tbl, int num_of_cmds);

/* disasm functions */
unsigned char *store_code (FILE *code_file);
size_t estimate_prog_size (unsigned char *code_buffer, size_t code_buffer_size);
int disassemble_code (unsigned char *code_buffer, size_t code_buffer_size, FILE *prog_file);
void upload_prog (char *prog_buffer, FILE *prog_file, size_t disassembled_cmds_size);
void clean_disasm_memory (unsigned char *code_buffer, char *prog_buffer);

#endif

/*

валгринд
declared static but never defined

не хватает мейка

добавить фиксированную запятую
поменять буферизованный ввод через fread на небуферизованный read

норм ли ограничивать набор символов, используемых в коде? или пора расчехлять isblank вместо == ' '?

*/