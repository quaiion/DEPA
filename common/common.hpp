#ifndef COMMON_ACTIVE
#define COMMON_ACTIVE

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <cstdint>
#include <time.h>

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
constexpr int CONSOLE_SIZE_VERT = 35;
constexpr int CONSOLE_SIZE_HOR = 162;
constexpr int VIDEOSEG_SIZE = CONSOLE_SIZE_HOR * CONSOLE_SIZE_VERT /* = 5670 pixels in total */;

/* These constants should NOT be changed */
constexpr int MAX_LABEL_NAME_BYTE_SIZE = MAX_CMD_NAME_BYTE_SIZE - sizeof (char);
constexpr int MIN_LABEL_NAME_BYTE_SIZE = MIN_CMD_NAME_BYTE_SIZE - sizeof (char);

constexpr unsigned char NOT_A_CMD = 0x1f;
constexpr int UNKNOWN_MARK = -1;

constexpr int MAX_NUM_OF_CMD_TYPES = 31;

struct code_info_t {

    int sig;
    int arch;
};

struct stop_signal_t {        // Утилитарная структура, являющая собой стандартный инструмент для условной остановки программы из макроса-инструкции

    bool stop;
    bool err;
};

constexpr unsigned char REG_BIT_MASK = 128;
constexpr unsigned char IMM_BIT_MASK = 64;
constexpr unsigned char RAM_BIT_MASK = 32;
constexpr unsigned char ONLY_CMD_TYPE_MASK = 31;

enum CMD_NAME_CNSTS {
#define CMD_PATTERN(token, arg_extraction_alg, arg_byte_size, execution_alg, preasm_format_alg, extern_arg, arg_assem_alg, arg_disas_print, max_disasm_arg_len) token , 
#include "../common/commands.hpp"
#undef CMD_PATTERN
NUM_OF_CMD_TYPES
};

size_t get_file_size (FILE *file);
int get_num_of_digits_int (int number);
int get_num_of_digits_sizet (size_t number);
int int_abs (int num);
int convert_float_to_int_1000 (float num_fl);
float convert_int_1000_to_float (int num_int);
char *search_digit (char *str);
int verify_cmd_num ();
int verify_videomem ();
int verify_reg_num (int reg);

#endif
