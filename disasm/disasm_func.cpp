#include "disasm.hpp"

static char *resize_prog_buffer (char *prog_buffer, size_t buffer_size, size_t buffer_cap);
static int verify_disasm_code_signature (unsigned char *code_buffer);

void verify_disasm_launch_parameters (int argc) {

    if (! (argc == 2 || argc == 3)) {

        printf ("\nWrong input, please insert codefile name and then (additionally) program file name only\n");
        exit (EXIT_FAILURE);
    }
}

static int verify_disasm_code_signature (unsigned char *code_buffer) {

    assert (code_buffer);

    if ((*((code_info_t *) code_buffer)).sig != 'QO') {

        printf ("\nDISASSEMBLING FAULT: INVALID FILE TYPE\n");
        return ERROR;
    }

    if ((*((code_info_t *) code_buffer)).arch != 'CISC') {

        printf ("\nDISASSEMBLING FAULT: INCONGRUENT CODE ARCHITECTURE\n");
        return ERROR;
    }

    return SUCCESS;
}

FILE *open_prog_file (int argc, char *argv []) {

    assert (argv);

    return (argc == 2) ? (fopen ("QO_prog_file.asm", "w")) : (fopen (argv [2], "w"));
}

unsigned char *store_code (FILE *code_file, size_t *code_buffer_size) {

    assert (code_buffer_size);
    assert (code_file);

    size_t code_file_size = get_file_size (code_file);
    
    unsigned char *code_buffer = (unsigned char *) calloc (code_file_size, sizeof (unsigned char));
    if (code_buffer == NULL) {

        printf ("\nLOADING FAULT: MEMORY ERROR\n");
        exit (EXIT_FAILURE);
    }

    fread (code_buffer, sizeof (char), code_file_size, code_file);
    rewind (code_file);

    *code_buffer_size = code_file_size - sizeof (code_info_t);
    code_buffer += sizeof (code_info_t);

    return code_buffer;
}

int disassemble_code (unsigned char* code_buffer, size_t code_buffer_size, char **prog_buffer_ptr, size_t prog_buffer_size, size_t *disassembled_cmds_size) {

    assert (code_buffer);
    assert (prog_buffer_ptr);
    assert (disassembled_cmds_size);

    char *prog_buffer = *prog_buffer_ptr;

    if (verify_disasm_code_signature (code_buffer - sizeof (code_info_t))) {

        return ERROR;
    }

    memset (prog_buffer, '\0', prog_buffer_size);
    size_t prog_buffer_cap = prog_buffer_size;
    stop_signal_t signal = {};

    char *prog_ptr = prog_buffer;
    for (size_t bytes_handled = 0; bytes_handled < code_buffer_size; ++ bytes_handled) {

        switch (code_buffer [bytes_handled] & ONLY_CMD_TYPE_MASK) {

#define CMD_PATTERN(name_cnst, token, arg_extraction_alg, arg_byte_size, execution_alg, preasm_format_alg, extern_arg, arg_assem_alg, arg_disas_print, max_disasm_arg_len) \
            case name_cnst: { \
                \
                prog_buffer = resize_prog_buffer (prog_buffer, (prog_ptr - prog_buffer) + \
                                                  sizeof (##token) + max_disasm_arg_len + 2 * sizeof (char), *prog_buffer_cap); \
                if (prog_buffer == NULL) { \
                    \
                    printf ("\nDISASSEMBLING FAULT: MEMORY ERROR\n"); \
                    signal.stop = true; \
                    signal.err = true; \
                    break; \
                } \
                \
                strcpy (prog_ptr, ##token); \
                prog_ptr = strchr (prog_ptr, '\0'); \
                \
                char *arg_str = (char *) calloc (max_disasm_arg_len, sizeof (char)); \
                unsigned char *code_ptr = code_buffer + bytes_handled; \
                arg_extraction_alg \
                if (signal.stop) { \
                    \
                    break; \
                } \
                arg_disas_print \
                if (signal.stop) { \
                    \
                    break; \
                } \
                \
                if ((arg_str != '\0')) { \
                    \
                    *prog_ptr = ' '; \
                    prog_ptr += sizeof (char); \
                    strncpy (prog_ptr, arg_str, max_disasm_arg_len); \
                    prog_ptr = strchr (prog_ptr, '\0'); \
                } \
                free (arg_str); \
                \
                *prog_ptr = '\n'; \
                prog_ptr += sizeof (char); \
                \
                bytes_handled += arg_byte_size; \
                break; \
            }

#include "../common/commands.hpp"
#undef CMD_PATTERN

            default: {

                printf ("\nbyte %llu: DISASSEMBLING FAULT: UNKNOWN COMMAND CODE\n", bytes_handled + sizeof (code_info_t));
                return ERROR;
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

    *disassembled_cmds_size = (size_t) (prog_ptr - prog_buffer);
    return SUCCESS;
}

static char *resize_prog_buffer (char *prog_buffer, size_t expected_buf_size, size_t *buf_cap) {

    while (expected_buf_size >= *buf_cap) {

        size_t old_cap = *buf_cap;
        *buf_cap *= 2;
        prog_buffer = (char *) realloc (prog_buffer, *buf_cap);
        if (prog_buffer) {

            memset (prog_buffer + old_cap, '\0', old_cap);
        }
    }

    return prog_buffer;
}

void upload_prog (char *prog_buffer, FILE *prog_file, size_t disassembled_cmds_size) {

    assert (prog_buffer);
    assert (prog_file);

    fwrite (prog_buffer, sizeof (char), disassembled_cmds_size, prog_file);
}

void clean_disasm_memory (unsigned char *code_buffer, char *prog_buffer) {

    if (code_buffer) {

        free (code_buffer - sizeof (code_info_t));
    }
    if (prog_buffer) {
        
        free (prog_buffer);
    }
}
