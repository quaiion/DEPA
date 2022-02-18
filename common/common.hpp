#ifndef COMMON_ACTIVE
#define COMMON_ACTIVE

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define NO_OFFSET 0
#define STRINGS_EQUAL 0
#define ERROR -1
#define SUCCESS 0

/* These constants can be adjusted */
constexpr int NUM_OF_REGS = 4;
constexpr int RAM_SIZE = 1048576;
constexpr int MAX_ARG_BYTES_LISTED = 10;
constexpr int MAX_CMD_LINE_LISTED = 16;
constexpr int MAX_CMD_NAME_BYTE_SIZE = 8;
constexpr int MIN_CMD_NAME_BYTE_SIZE = 2;

constexpr int VIDEOSEG_START = 0x800;
constexpr int CONSOLE_SIZE_VERT = 36;
constexpr int CONSOLE_SIZE_HOR = 162;
constexpr int VIDEOSEG_SIZE = CONSOLE_SIZE_HOR * CONSOLE_SIZE_VERT /* = 5832 bytes in total */;

/* These constants should NOT be changed */
constexpr int MAX_MARK_NAME_BYTE_SIZE = MAX_CMD_NAME_BYTE_SIZE - sizeof (char);
constexpr int MIN_MARK_NAME_BYTE_SIZE = MIN_CMD_NAME_BYTE_SIZE - sizeof (char);

constexpr int NOT_A_CMD = -1;
constexpr int UNKNOWN_MARK = -1;

struct code_info_t {

    int sig;
    int arch;
};

struct stop_signal_t {        // Утилитарная структура, являющая собой стандартный инструмент для условной остановки программы из макроса-инструкции

    bool stop;
    bool err;
};

constexpr char REG_BIT_MASK = 128;
constexpr char IMM_BIT_MASK = 64;
constexpr char RAM_BIT_MASK = 32;
constexpr char ONLY_CMD_TYPE_MASK = 31;

enum CMD_NAME_CNSTS {
#define CMD_PATTERN(name_cnst, token, arg_extraction_alg, arg_byte_size, execution_alg, preasm_format_alg, extern_arg, arg_assem_alg, arg_disas_print, max_disasm_arg_len) name_cnst , 
#include "../common/commands.hpp"
#undef CMD_PATTERN
NUM_OF_CMD_TYPES
};

enum CMD_NAME_CNSTS {HLT = 0, PUSH = 1, POP = 2, ADD = 3, SUB = 4, MUL = 5, DIV = 6, IN = 7, OUT = 8, STDMP = 9, STVRF = 10, RGDMP = 11,
                     JMP = 12, JA = 13, JAE = 14, JB = 15, JBE = 16, JE = 17, JNE = 18, JF = 19, CALL = 20, RET = 21, ASTDMP = 22, ASTVRF = 23};

constexpr int NUM_OF_CMD_TYPES = 24;

size_t get_file_size (FILE *file);
int get_num_of_digits_int (int number);
int get_num_of_digits_sizet (size_t number);
int int_abs (int num);
int convert_float_to_int_1000 (float num_fl);
float convert_int_1000_to_float (int num_int);
int lin_search_str (char *cmd);
char *search_digit (char *str);

#endif

/*

check пару исключений - отсутствие файла
с отрицательными числами точно есть какие-то траблы (см. листинг)
нули в дампе

*/
