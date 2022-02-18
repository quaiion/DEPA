#include "asm.hpp"

static int verify_reg_name (unsigned long line, char reg);
static int verify_cmd_end_format (unsigned long line, char end_symb_1, char end_symb_2);
static int verify_cmd_name (unsigned long line, cmd_idx_t *cmd);
static int verify_cmd_space_format (unsigned long line, cmd_idx_t *cmd);
static int verify_jump_format (unsigned long line, cmd_idx_t *cmd);
static int verify_unknown_mark (unsigned long line, int mark_num);
static int verify_arg_num (unsigned long line, float num);
static int verify_cmd_arg_format (unsigned long line, int format);
static int lin_search_mrk (char *mark, mark_t *mark_tbl, int nmarks);
static void upload_listing (char *listing_buffer, size_t listing_size);

void verify_asm_launch_parameters (int argc) {

    if (! (argc == 2 || argc == 3)) {

        printf ("\nWrong input, please insert program file name and then (additionally) codefile name only\n");
        exit (EXIT_FAILURE);
    }
}

FILE *open_code_file (int argc, char *argv []) {

    assert (argv);

    return (argc == 2) ? (fopen ("QO_code_file.bin", "wb")) : (fopen (argv [2], "wb"));
}

char *store_cmds (FILE *prog_file) {

    assert (prog_file);

    size_t prog_file_size = get_file_size (prog_file);
    
    char *cmd_buffer = (char *) calloc (prog_file_size + 1, sizeof (char));
    if (cmd_buffer == NULL) {

        printf ("\nLOADING FAULT: MEMORY ERROR\n");
        exit (EXIT_FAILURE);
    }

    size_t bytes_read = fread (cmd_buffer, sizeof (char), prog_file_size, prog_file);
    cmd_buffer [bytes_read] = '\0';
    rewind (prog_file);

    return cmd_buffer;
}

unsigned long get_num_of_cmds (char *cmd_buffer) {

    assert (cmd_buffer);

    unsigned long num_of_cmds = 0;
    char *cmd_end = cmd_buffer - sizeof (char), *cmd_beg = NULL;
    for ( ; cmd_end != NULL; ++ num_of_cmds) {

        cmd_beg = cmd_end + sizeof (char);
        cmd_end = strchr (cmd_beg, '\n');
    }

    if (*(cmd_beg) == '\0' && num_of_cmds != 0) {

        num_of_cmds -= 1;
    }

    return num_of_cmds;
}

cmd_idx_t *index_cmds (char *cmd_buffer, unsigned long num_of_cmds) {

    assert (cmd_buffer);

    cmd_idx_t *cmd_index_tbl = (cmd_idx_t *) calloc (num_of_cmds, sizeof (cmd_idx_t));
    if (cmd_index_tbl == NULL) {

        printf ("\nASSEMBLING FAULT: MEMORY ERROR\n");
        exit (EXIT_FAILURE);
    }

    cmd_index_tbl [0].line = cmd_buffer;
    cmd_buffer = strchr (cmd_buffer, '\n');
    for (int i = 1; i < num_of_cmds; ++ i) {

        *cmd_buffer = '\0';
        cmd_index_tbl [i].line = cmd_buffer + sizeof (char);
        cmd_buffer = strchr (cmd_buffer + sizeof (char), '\n');
    }

    if (cmd_buffer != NULL) {

        *cmd_buffer = '\0';
    }

    for (int i = 0; i < num_of_cmds; ++ i) {

        cmd_index_tbl [i]->space = strchr (cmd_index_tbl [i].line, ' ');
    }

    return cmd_index_tbl;
}

void set_code_info (unsigned char **code_buffer) {

    assert (code_buffer);

    code_info_t code_info = {};
    code_info.sig = 'QO';
    code_info.arch = 'CISC';

    *((code_info_t *) *code_buffer) = code_info;

    *code_buffer += sizeof (code_info_t);
}

static int verify_reg_name (unsigned long line, char reg) {

    if (reg < 'a' || reg > 'a' + NUM_OF_REGS - 1) {

        printf ("\nline %lu: ASSEMBLING FAULT: WRONG REGISTER NAME\n", line);
        return ERROR;
    }

    return SUCCESS;
}

static int verify_cmd_end_format (unsigned long line, char end_symb_1, char end_symb_2) {

    if (end_symb_1 != '\0' && !(end_symb_1 == ' ' && end_symb_2 == '/')) {

        printf ("\nline %lu: ASSEMBLING FAULT: WRONG FORMAT\n", line);
        return ERROR;
    }

    return SUCCESS;
}

static int verify_cmd_name (unsigned long line, cmd_idx_t *cmd) {

    if (cmd->space == NULL) {

        if (strlen (cmd->line) > MAX_CMD_NAME_BYTE_SIZE) {

            printf ("\nline %lu: ASSEMBLING FAULT: IMPOSSIBLE COMMAND NAME (ALLOWED NAME LENGTH EXCEEDED)\n", line);
            return ERROR;
        }

        if (strlen (cmd->line) < MIN_CMD_NAME_BYTE_SIZE) {

            printf ("\nline %lu: ASSEMBLING FAULT: IMPOSSIBLE COMMAND NAME (TOO SHORT NAME)\n", line);
            return ERROR;
        }
    } else {

        if (cmd->space - cmd->line > MAX_CMD_NAME_BYTE_SIZE) {

            printf ("\nline %lu: ASSEMBLING FAULT: IMPOSSIBLE COMMAND NAME (ALLOWED NAME LENGTH EXCEEDED)\n", line);
            return ERROR;
        }
        
        if (cmd->space - cmd->line < MIN_CMD_NAME_BYTE_SIZE) {

            printf ("\nline %lu: ASSEMBLING FAULT: IMPOSSIBLE COMMAND NAME (TOO SHORT NAME)\n", line);
            return ERROR;
        }
    }

    return SUCCESS;
}

static int verify_cmd_space_format (unsigned long line, cmd_idx_t *cmd) {

    if (cmd->line [0] == ' ' ||
       (cmd->space != NULL && *(cmd->space + sizeof (char)) == ' ')) {

        printf ("\nline %lu: ASSEMBLING FAULT: WRONG FORMAT\n", line);
        return ERROR;
    }

    return SUCCESS;
}

static int verify_jump_format (unsigned long line, cmd_idx_t *cmd) {

    if (cmd->space == NULL) {

        printf ("\nline %lu: ASSEMBLING FAULT: WRONG FORMAT\n", line);
        return ERROR;
    }

    if (*(cmd->space + sizeof (char)) == '/') {

        printf ("\nline %lu: ASSEMBLING FAULT: WRONG FORMAT\n", line);
        return ERROR;
    }

    char *next_space = strchr (cmd->space + sizeof (char), ' ');
    char *line_end = strchr (cmd->space + sizeof (char), '\0');

    if (next_space == NULL) {

        if (line_end - (cmd->space + sizeof (char)) < MIN_MARK_NAME_BYTE_SIZE) {

            printf ("\nline %lu: ASSEMBLING FAULT: IMPOSSIBLE MARK NAME (TOO SHORT NAME)\n", line);
            return ERROR;
        }

        if (line_end - (cmd->space + sizeof (char)) > MAX_MARK_NAME_BYTE_SIZE) {

            printf ("\nline %lu: ASSEMBLING FAULT: IMPOSSIBLE MARK NAME (ALLOWED NAME LENGTH EXCEEDED)\n", line);
            return ERROR;
        }

    } else {

        if (next_space - (cmd->space + sizeof (char)) < MIN_MARK_NAME_BYTE_SIZE) {

            printf ("\nline %lu: ASSEMBLING FAULT: IMPOSSIBLE MARK NAME (TOO SHORT NAME)\n", line);
            return ERROR;
        }

        if (next_space - (cmd->space + sizeof (char)) > MAX_MARK_NAME_BYTE_SIZE) {

            printf ("\nline %lu: ASSEMBLING FAULT: IMPOSSIBLE MARK NAME (ALLOWED NAME LENGTH EXCEEDED)\n", line);
            return ERROR;
        } 
    }

    return SUCCESS;
}

static int verify_unknown_mark (unsigned long line, int mark_num) {

    if (mark_num == UNKNOWN_MARK) {

        printf ("\nline %lu: ASSEMBLING FAULT: UNKNOWN MARK\n", line);
        return ERROR;
    }

    return SUCCESS;
}

static int verify_arg_num (unsigned long line, float num) {

    if (((float) INT_MAX) / 1000 < num) {

        printf ("\nline %lu: ASSEMBLING FAULT: MAX NUMBER ARGUMENT VALUE EXCEEDED\n", line);
        return ERROR;
    }

    return SUCCESS;
}

static int verify_cmd_arg_format (unsigned long line, int format) {

    if (format == 0) {

        printf ("\nline %lu: ASSEMBLING FAULT: WRONG FORMAT\n", line);
        return ERROR;
    }

    return SUCCESS;
}

int preassemble_prog (cmd_idx_t *cmd_index_tbl, unsigned long num_of_cmds, mark_t **mark_tbl, int *num_of_marks, size_t *code_size) {

    assert (cmd_index_tbl);

    *mark_tbl = (mark_t *) calloc (DEFAULT_MARK_TABLE_ELEM_SIZE, sizeof (mark_t));
    if (*mark_tbl == NULL) {

        printf ("\nASSEMBLING FAULT: MEMORY ERROR\n");
        exit (EXIT_FAILURE);
    }
    int mark_tbl_size = DEFAULT_MARK_TABLE_ELEM_SIZE;

    int marks_handled = 0;
    size_t handled_code_size = 0;
    for (unsigned long cmds_handled = 0; cmds_handled < num_of_cmds; ++cmds_handled) {

        char *line = cmd_index_tbl [cmds_handled].line;
        char *space = cmd_index_tbl [cmds_assembled].space;
        assert (line);

        char cmd [MAX_CMD_NAME_BYTE_SIZE + sizeof (char)] = {};

        if (line [0] == '/') {

            cmd_index_tbl [cmds_handled].cmd_cnst = NOT_A_CMD;
            continue;
        }
        if (verify_cmd_space_format (cmds_handled + 1, cmd_index_tbl + cmds_handled)) {

            return ERROR;
        }
        if (verify_cmd_name (cmds_handled + 1, cmd_index_tbl + cmds_handled)) {

            return ERROR;
        }

        sscanf (line, "%s", cmd);

        do {
#define CMD_PATTERN(name_cnst, token, arg_extraction_alg, arg_byte_size, execution_alg, preasm_format_alg, extern_arg, arg_assem_alg, arg_disas_print, max_disasm_arg_len) \
            if (strcmp (cmd, #token) == STRINGS_EQUAL) { \
                \
                cmd_index_tbl [cmds_handled].cmd_cnst = name_cnst; \
                break; \
            }

#include "../common/commands.hpp"
#undef CMD_PATTERN

        } while (0);

        stop_signal_t signal = {};

        switch (cmd_index_tbl [cmds_handled].cmd_cnst) {

#define CMD_PATTERN(name_cnst, token, arg_extraction_alg, arg_byte_size, execution_alg, preasm_format_alg, extern_arg, arg_assem_alg, arg_disas_print, max_disasm_arg_len) \
            case name_cnst: { \
                \
                preasm_format_alg \
                if (signal.stop) { \
                    \
                    break; \
                } \
                \
                handled_code_size += arg_byte_size + sizeof (unsigned char); \
                \
                break; \
            }

#include "../common/commands.hpp"
#undef CMD_PATTERN

            default: {

                if (cmd [cmd_len - 1] == ':') {

                    if (verify_cmd_end_format (cmds_handled + 1, line [cmd_len], line [cmd_len + 1])) {

                        return ERROR;
                    }

                    if (marks_handled == mark_tbl_size) {

                        *mark_tbl = (mark_t *) realloc (*mark_tbl, mark_tbl_size * sizeof (mark_t) * 2);
                        mark_tbl_size *= 2;
                    }

                    cmd [cmd_len - 1] = '\0';
                    strcpy (((*mark_tbl) [marks_handled]).name, cmd);
                    ((*mark_tbl) [marks_handled]).idx = handled_code_size;
                    marks_handled += 1;

                    break;

                } else {

                    printf ("\nline %lu: ASSEMBLING FAULT: UNKNOWN COMMAND\n", cmds_handled + 1);
                    return ERROR;
                }
            }
        }

        if (signal.stop) {

            if (signal.err) {

                return ERROR;

            } else {

                break;
            }
        }
    }

    *code_size = handled_code_size;
    *num_of_marks = marks_handled;
    return SUCCESS;
}

constexpr int listing_initial_offset = 15;
constexpr int listing_arg_offset = 3;

int assemble_prog (cmd_idx_t *cmd_index_tbl, unsigned long num_of_cmds, unsigned char *code_buffer, mark_t *mark_tbl, int num_of_marks) {

    assert (cmd_index_tbl);
    assert (code_buffer);
    assert (num_of_marks >= 0);

    char *listing_buffer = (char *) calloc (num_of_cmds * MAX_LISTING_LINE_SIZE, sizeof (char));
    if (listing_buffer == NULL) {

        printf ("\nASSEMBLING FAULT: MEMORY ERROR\n");
        exit (EXIT_FAILURE);
    }
    char *listing_ptr = listing_buffer;

    size_t handled_code_size = 0;
    for (unsigned long cmds_assembled = 0; cmds_assembled < num_of_cmds; cmds_assembled ++) {

        assert (cmd_index_tbl [cmds_assembled].line);

        if (cmd_index_tbl [cmds_assembled].cmd_cnst == NOT_A_CMD) {

            continue;
        }

        char cmd_cnst = cmd_index_tbl [cmds_assembled].cmd_cnst;
        char *space = cmd_index_tbl [cmds_assembled].space;
        char reg = 0, end_symb_1 = 0, end_symb_2 = 0;
        float num_fl = 0;
        int arg = 0;
        int format = 0;
        stop_signal_t signal = {};

        switch (cmd_cnst & ONLY_CMD_TYPE_MASK) {

#define CMD_PATTERN(name_cnst, token, arg_extraction_alg, arg_byte_size, execution_alg, preasm_format_alg, extern_arg, arg_assem_alg, arg_disas_print, max_disasm_arg_len) \
            case name_cnst: { \           
                \
                sprintf (listing_ptr, "\n%.8X   %.2X ", handled_code_size + sizeof (code_info_t), cmd_cnst); \
                listing_ptr += listing_initial_offset; \
                \
                code_buffer [handled_code_size ++] = cmd_cnst; \
                arg_assem_alg \
                if (signal.stop) { \
                    \
                    break; \
                } \
                \
                int arg_bytes_listed = 0; \
                for ( ; arg_bytes_listed < arg_byte_size; ++ arg_bytes_listed) { \
                    \
                    sprintf (listing_ptr, " %.2X", code_buffer [handled_code_size + arg_bytes_listed]); \
                    listing_ptr += listing_arg_offset; \
                } \
                for ( ; arg_bytes_listed < MAX_ARG_BYTES_LISTED; ++ arg_bytes_listed) { \
                    \
                    sprintf (listing_ptr, "   "); \
                    listing_ptr += listing_arg_offset; \
                } \
                \
                if (extern_arg) { \
                    \
                    if (space == NULL) { \
                        \
                        end_symb_1 = '\0'; \
                        \
                    } else { \
                        \
                        end_symb_1 = *space; \
                        end_symb_2 = *(space + 1); \
                    } \
                } \
                \
                if (verify_cmd_end_format (cmds_assembled + 1, end_symb_1, end_symb_2)) { \
                    \
                    asm_err = true; \
                    break; \
                } \
                \
                sprintf (listing_ptr, "   %s", cmd_index_tbl [cmds_assembled].line); \
                listing_ptr += strlen (cmd_index_tbl [cmds_assembled].line); \
                \
                handled_code_size += arg_byte_size; \
                break; \
            }

#include "../common/commands.hpp"
#undef CMD_PATTERN

            default: {

                printf ("\nline %lu: ASSEMBLING FAULT: PROCESS INTERRUPTED\n", cmds_assembled + 1);
                free (listing_buffer);
                return ERROR;
            }
        }

        if (signal.stop) {

            if (signal.err) {

                free (listing_buffer);
                return ERROR;

            } else {

                break;
            }
        }
    }

    upload_listing (listing_buffer, symbs_listed);
    free (listing_buffer);

    return SUCCESS;
}

static void upload_listing (char *listing_buffer, size_t listing_size) {

    FILE *listing_file = fopen ("assembling_report.lst", "w");
    fwrite (listing_buffer, sizeof (char), listing_size, listing_file);
    fclose (listing_file);
}

void upload_code (unsigned char *code_buffer, FILE *code_file, size_t assembled_code_size) {

    assert (code_buffer);
    assert (code_file);

    fwrite (code_buffer - sizeof (code_info_t), sizeof (unsigned char), assembled_code_size + sizeof (code_info_t), code_file);
}

void clean_asm_memory (cmd_idx_t *cmd_index_tbl, char *cmd_buffer, unsigned char *code_buffer, mark_t *mark_tbl) {

    if (cmd_buffer) {

        free (cmd_buffer);
    }
    if (cmd_index_tbl) {

        free (cmd_index_tbl);
    }
    if (mark_tbl) {
        
        free (mark_tbl);
    }
    if (code_buffer) {

        free (code_buffer - sizeof (code_info_t));
    }
}

static int lin_search_mrk (char *mark, mark_t *mark_tbl, int nmarks) {

    for (int i = 0; i < nmarks; ++i) {

        if (strcmp (mark, mark_tbl [i].name) == STRINGS_EQUAL) {

            return i;
        }
    }

    return UNKNOWN_MARK;
}
