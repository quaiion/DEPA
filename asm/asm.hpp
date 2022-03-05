#ifndef ASM_ACTIVE
#define ASM_ACTIVE

#include "../common/common.hpp"

constexpr int MAX_LISTING_LINE_SIZE = 18 + 3 * MAX_ARG_BYTES_LISTED + MAX_CMD_LINE_LISTED;
constexpr unsigned DEFAULT_MARK_TABLE_ELEM_SIZE = 8;

struct mark_t {

    char name [MAX_MARK_NAME_BYTE_SIZE + sizeof (char)];
    size_t idx;
};

struct cmd_idx_t {

    char *line;
    char *space;
    unsigned char cmd_cnst;
};

FILE *open_code_file (int argc, char *argv []);
int assemble_prog (cmd_idx_t *cmd_index_tbl, unsigned long num_of_cmds, unsigned char *code_buffer, mark_t *mark_tbl, unsigned num_of_marks);
int preassemble_prog (cmd_idx_t *cmd_index_tbl, unsigned long num_of_cmds, mark_t **mark_tbl, unsigned *num_of_marks, size_t *code_size);
void set_code_info (unsigned char **code_buffer);
char *store_cmds (FILE *prog_file);
void upload_code (unsigned char *code_buffer, FILE *code_file, size_t assembled_code_size);
cmd_idx_t *index_cmds (char *cmd_buffer, unsigned long num_of_cmds);
void clean_asm_memory (cmd_idx_t *cmd_index_tbl, char *cmd_buffer, unsigned char *code_buffer, mark_t *mark_tbl);
unsigned long get_num_of_cmds (char *cmd_buffer);
int verify_asm_launch_parameters (int argc);

#endif
