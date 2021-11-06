#ifndef CPU_ACTIVE
#define CPU_ACTIVE

#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include "stack.hpp"

#define NO_OFFSET 0
#define STRINGS_EQUAL 0
#define ERROR -1
#define SUCCESS 0

constexpr int SIGNATURE = 'QO';

constexpr int NUM_OF_REGS = 4;
constexpr int RAM_SIZE = 1048576;

struct cpu_t {

    unsigned char *code;
    int arch;
    int *ram;
    int reg [NUM_OF_REGS];
    stack_t stack;
    size_t ip;
    size_t code_size;
};

struct code_info_t {

    int sig;
    int arch;
};

constexpr char REG_BIT_MASK = 128;
constexpr char IMM_BIT_MASK = 64;
constexpr char RAM_BIT_MASK = 32;
constexpr char ONLY_CMD_TYPE_MASK = 31;

enum CMD_NAME_CNSTS {HLT = 0, PUSH = 1, POP = 2, ADD = 3, SUB = 4, MUL = 5, DIV = 6, IN = 7, OUT = 8, STDMP = 9, STVRF = 10};

const char *CMD_NAMES [] = {"hlt" /* 0 */, "push" /* 1 */, "pop" /* 2 */, "add" /* 3 */, "sub" /* 4 */, "mul" /* 5 */,        \
                            "div" /* 6 */, "in" /* 7 */, "out" /* 8 */, "stdmp" /* 9 */, "stvrf" /* 10 */};

constexpr unsigned char NUM_OF_CMD_TYPES = 11;
constexpr int MAX_CMD_NAME_BYTE_SIZE = 6;

/* cpu functions */
void cpu_ctor (cpu_t *cpu);
void cpu_dtor (cpu_t *cpu);
void load_code (FILE *code_file, cpu_t *cpu);
int execute_code (cpu_t *cpu);

/* asm functions */
int assemble_prog (char **cmd_index_tbl, int num_of_cmds, FILE *code_file);
char* store_cmds (FILE *prog_file);
char **index_cmds (char *cmd_buffer, int num_of_cmds);
void clean_memory (char **cmd_index_tbl, char *cmd_buffer);
FILE *open_code_file (int argc, char *argv []);
int get_num_of_cmds (char *cmd_buffer);

/* disasm functions */
FILE *open_prog_file (int argc, char *argv []);
unsigned char *store_code (FILE *code_file);
int disassemble_code (unsigned char *code_buffer, size_t code_buffer_size, FILE *prog_file);

#endif

/*

Валгринд
declared static but never defined

Не хватает листинга, мейка
верификация пустого кода
команда rgdmp

добавить фиксированную запятую
поменять буферизованный ввод через fread на небуферизованный read

*/