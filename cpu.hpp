#ifndef CPU_ACTIVE
#define CPU_ACTIVE

#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

struct code_info_t {

    int sig;
    int arch;
};

constexpr int NUM_OF_REGS = 4;
constexpr int RAM_SIZE = 1048576;

constexpr int SIG_QO = 'QO';
constexpr int ARCH_CISC = 'CISC';

constexpr char REG_BIT_MASK = 128;
constexpr char IMM_BIT_MASK = 64;
constexpr char RAM_BIT_MASK = 32;

constexpr char ONLY_CMD_TYPE_MASK = 31;

enum CMD_NAME_CNSTS {HLT = 0, PUSH = 1, POP = 2, ADD = 3, SUB = 4, MUL = 5, DIV = 6, IN = 7, OUT = 8, STDMP = 9, STVRF = 10};

const char *CMD_NAMES [] = {"hlt" /* 0 */, "push" /* 1 */, "pop" /* 2 */, "add" /* 3 */, "sub" /* 4 */, "mul" /* 5 */,        \
                            "div" /* 6 */, "in" /* 7 */, "out" /* 8 */, "stdmp" /* 9 */, "stvrf" /* 10 */};

constexpr unsigned char NUM_OF_CMD_TYPES = 11;
constexpr int MAX_CMD_NAME_BYTE_SIZE = 6;
constexpr int STRINGS_EQUAL = 0;

#endif

/*

1. Проблемы с файлом кода и изменениями при передаче
2. Проблемы с подключением стека
3. Валгринд

Не хватает листинга, мейка
верификация пустого кода

добавить фиксированную запятую
поменять буферизованный ввод через fread на небуферизованный read

*/