#ifndef COMMON_ACTIVE
#define COMMON_ACTIVE

#include <stdio.h>
#include <assert.h>

#define NO_OFFSET 0
#define STRINGS_EQUAL 0
#define ERROR -1
#define SUCCESS 0

constexpr int NUM_OF_REGS = 4;
constexpr int RAM_SIZE = 1048576;

constexpr int MAX_CMD_NAME_BYTE_SIZE = 8;
constexpr int MIN_CMD_NAME_BYTE_SIZE = 2;

/* These constants should NOT be changed */
constexpr int MAX_MARK_NAME_BYTE_SIZE = MAX_CMD_NAME_BYTE_SIZE - sizeof (char);
constexpr int MIN_MARK_NAME_BYTE_SIZE = MIN_CMD_NAME_BYTE_SIZE - sizeof (char);

struct code_info_t {

    int sig;
    int arch;
};

constexpr char REG_BIT_MASK = 128;
constexpr char IMM_BIT_MASK = 64;
constexpr char RAM_BIT_MASK = 32;
constexpr char ONLY_CMD_TYPE_MASK = 31;

enum CMD_NAME_CNSTS {HLT = 0, PUSH = 1, POP = 2, ADD = 3, SUB = 4, MUL = 5, DIV = 6, IN = 7, OUT = 8, STDMP = 9, STVRF = 10, RGDMP = 11,            \
                     JMP = 12, JA = 13, JAE = 14, JB = 15, JBE = 16, JE = 17, JNE = 18, JF = 19, CALL = 20, RET = 21, ASTDMP = 22, ASTVRF = 23};

// const char *CMD_NAMES [] = {"hlt" /* 0 */, "push" /* 1 */, "pop" /* 2 */, "add" /* 3 */, "sub" /* 4 */, "mul" /* 5 */,              \
//                             "div" /* 6 */, "in" /* 7 */, "out" /* 8 */, "stdmp" /* 9 */, "stvrf" /* 10 */, "rgdmp" /* 11 */,        \
//                             "jmp" /* 12 */, "ja" /* 13 */, "jae" /* 14 */, "jb" /* 15 */, "jbe" /* 16 */, "je" /* 17 */,            \
//                             "jne" /* 18 */, "jf" /* 19 */, "call" /* 20 */, "ret" /* 21 */, "astdmp" /* 22 */, "astvrf" /* 23 */};

constexpr unsigned char NUM_OF_CMD_TYPES = 24;

size_t get_file_size (FILE *file);
int get_num_of_digits_int (int number);
int get_num_of_digits_sizet (size_t number);
int int_abs (int num);
int convert_float_to_int_1000 (float num_fl);
float convert_int_1000_to_float (int num_int);

#endif

/*

утилиты работы с таблицей меток по хорошему надо в отдельные функи разнести
не хватает мейка
check пару исключений - отсутствие файла и странности с именами регистров
добавить фиксированную запятую
норм ли ограничивать набор символов, используемых в коде? или пора расчехлять isblank вместо == ' '?
правильная ли логика работы со статик функциями? в силу скомпановонности проекта их тут нет (в отличе от стека)
есть явные проблемы с переполнением типа в арифметических командах
с отрицательными числами точно есть какие-то траблы (см. листинг)
норм ли инклюдить коммон-хедер в частные хедеры?
нули в дампе

*/
