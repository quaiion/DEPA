#ifndef ASM_ACTIVE
#define ASM_ACTIVE

#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "common.hpp"

constexpr int MAX_LISTING_LINE_SIZE = 49;
constexpr int DEFAULT_MARK_TABLE_ELEM_SIZE = 8;

struct mark_t {

    char name [MAX_MARK_NAME_BYTE_SIZE + sizeof (char)];
    size_t idx;
};

FILE *open_code_file (int argc, char *argv []);
int assemble_prog (char **cmd_index_tbl, int num_of_cmds, unsigned char *code_buffer, size_t *assembled_code_size, mark_t *mark_tbl, int num_of_marks);
int preassemble_prog (char **cmd_index_tbl, int num_of_cmds, mark_t **mark_tbl, int *num_of_marks);
void set_code_info (unsigned char **code_buffer);
char *store_cmds (FILE *prog_file);
void upload_code (unsigned char *code_buffer, FILE *code_file, size_t assembled_code_size);
char **index_cmds (char *cmd_buffer, int num_of_cmds);
void clean_asm_memory (char **cmd_index_tbl, char *cmd_buffer, unsigned char *code_buffer, mark_t *mark_tbl);
int get_num_of_cmds (char *cmd_buffer);
size_t estimate_code_size (char **cmd_index_tbl, int num_of_cmds);
void upload_listing (char *listing_buffer, size_t listing_size);

void verify_asm_launch_parameters (int argc);
int verify_reg_name (int line, char reg);
int verify_cmd_end_format (int line, char end_symb1, char end_symb2);
int verify_unknown_cmd (int line, bool cmd_identified);
int verify_cmd_name (int line, char *cmd);
int verify_jump_format (int line, char *cmd);
int verify_cmd_space_format (int line, char *cmd);
int verify_arg_num (int line, float num);

#endif
