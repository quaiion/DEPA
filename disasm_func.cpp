#include "disasm.hpp"

// SHOULD BE DELETED:

const char *CMD_NAMES [] = {"hlt" /* 0 */, "push" /* 1 */, "pop" /* 2 */, "add" /* 3 */, "sub" /* 4 */, "mul" /* 5 */,              \
                            "div" /* 6 */, "in" /* 7 */, "out" /* 8 */, "stdmp" /* 9 */, "stvrf" /* 10 */, "rgdmp" /* 11 */,        \
                            "jmp" /* 12 */, "ja" /* 13 */, "jae" /* 14 */, "jb" /* 15 */, "jbe" /* 16 */, "je" /* 17 */,            \
                            "jne" /* 18 */, "jf" /* 19 */, "call" /* 20 */, "ret" /* 21 */, "astdmp" /* 22 */, "astvrf" /* 23 */};

// ;

void verify_disasm_launch_parameters (int argc) {

    if (! (argc == 2 || argc == 3)) {

        printf ("\nWrong input, please insert codefile name and then (additionally) program file name only\n");
        exit (EXIT_FAILURE);
    }
}

int verify_disasm_code_signature (unsigned char *code_buffer) {

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

int verify_cmd_code (size_t byte_number, unsigned char cmd_code) {

    if ((cmd_code & ONLY_CMD_TYPE_MASK) > NUM_OF_CMD_TYPES - 1) {

        printf ("\nbyte %llu: DISASSEMBLING FAULT: UNKNOWN COMMAND CODE\n", byte_number);
        return ERROR;
    }

    return SUCCESS;
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

    /*
    printf ("\n");
    for (size_t i = 0; i < *code_buffer_size + sizeof (code_info_t); i ++) {

        printf ("%d ", *((unsigned char *) (code_buffer + i - sizeof (code_info_t))));
    }
    printf ("\n");
    */

    return code_buffer;
}

size_t estimate_prog_size (unsigned char* code_buffer, size_t code_buffer_size) {

    assert (code_buffer);

    size_t max_cmds_size = 0;

    for (size_t bytes_handled = 0; bytes_handled < code_buffer_size; bytes_handled ++) {

        max_cmds_size += MAX_CMD_NAME_BYTE_SIZE + 3 * sizeof (char);

        if (code_buffer [bytes_handled] == STVRF || code_buffer [bytes_handled] == STDMP || code_buffer [bytes_handled] == RGDMP ||     \
            code_buffer [bytes_handled] == ASTVRF || code_buffer [bytes_handled] == ASTDMP) {

            bytes_handled += sizeof (int);
            continue;
        }

        if (code_buffer [bytes_handled] == JMP || code_buffer [bytes_handled] == JA || code_buffer [bytes_handled] == JAE ||   \
            code_buffer [bytes_handled] == JB || code_buffer [bytes_handled] == JBE || code_buffer [bytes_handled] == JE ||    \
            code_buffer [bytes_handled] == JNE || code_buffer [bytes_handled] == JF || code_buffer [bytes_handled] == CALL) {

            max_cmds_size += sizeof (char) * get_num_of_digits_sizet (*((size_t *) (code_buffer + bytes_handled + sizeof (unsigned char))));

            bytes_handled += sizeof (size_t);
            continue;
        }

        size_t bytes_diff = 0;

        if (code_buffer [bytes_handled] & RAM_BIT_MASK) {

            max_cmds_size += 2 * sizeof (char);
        }

        if (code_buffer [bytes_handled] & REG_BIT_MASK) {

            max_cmds_size += 2 * sizeof (char);
            bytes_diff += sizeof (unsigned char);
        }

        if (code_buffer [bytes_handled] & IMM_BIT_MASK) {

            max_cmds_size += sizeof (char) * (get_num_of_digits_int (*((int *) (code_buffer + bytes_handled + sizeof (unsigned char) + bytes_diff))) + 1);
            bytes_diff += sizeof (int);
        }

        bytes_handled += bytes_diff;
    }

    return max_cmds_size;
}

int disassemble_code (unsigned char* code_buffer, size_t code_buffer_size, char *prog_buffer, size_t *disassembled_cmds_size) {

    assert (code_buffer);
    assert (prog_buffer);
    assert (disassembled_cmds_size);

    if (verify_disasm_code_signature (code_buffer - sizeof (code_info_t))) {

        return ERROR;
    }

    /*
    printf ("\n");
    for (size_t i = 0; i < code_buffer_size + sizeof (code_info_t); i ++) {

        printf ("%d ", *((unsigned char *) (code_buffer + i - sizeof (code_info_t))));
    }
    printf ("\n");
    */

    size_t handled_cmds_size = 0;
    for (size_t bytes_handled = 0; bytes_handled < code_buffer_size; bytes_handled ++) {

        if (verify_cmd_code (bytes_handled + sizeof (code_info_t), code_buffer [bytes_handled])) {

            return ERROR;
        }

        sprintf (prog_buffer + handled_cmds_size, "%s", CMD_NAMES [code_buffer [bytes_handled] & ONLY_CMD_TYPE_MASK]);
        handled_cmds_size += strlen (CMD_NAMES [code_buffer [bytes_handled] & ONLY_CMD_TYPE_MASK]) * sizeof (char);

        if (code_buffer [bytes_handled] == STVRF || code_buffer [bytes_handled] == STDMP || code_buffer [bytes_handled] == RGDMP ||     \
            code_buffer [bytes_handled] == ASTVRF || code_buffer [bytes_handled] == ASTDMP) {

            *(prog_buffer + handled_cmds_size) = '\n';

            handled_cmds_size += (sizeof (char));
            bytes_handled += sizeof (int);
            continue;
        }

        if (code_buffer [bytes_handled] == JMP || code_buffer [bytes_handled] == JA || code_buffer [bytes_handled] == JAE ||   \
            code_buffer [bytes_handled] == JB || code_buffer [bytes_handled] == JBE || code_buffer [bytes_handled] == JE ||    \
            code_buffer [bytes_handled] == JNE || code_buffer [bytes_handled] == JF || code_buffer [bytes_handled] == CALL) {

            int arg = 0;
            arg = *((size_t *) (code_buffer + bytes_handled + sizeof (unsigned char)));
            sprintf (prog_buffer + handled_cmds_size, " %d", arg);
            handled_cmds_size += sizeof (char) * get_num_of_digits_int (arg) + sizeof (char);
            *(prog_buffer + handled_cmds_size) = '\n';
            handled_cmds_size += sizeof (char);

            bytes_handled += sizeof (size_t);
            continue;
        }

        size_t bytes_diff = 0;

        if ((code_buffer [bytes_handled] & REG_BIT_MASK) || (code_buffer [bytes_handled] & IMM_BIT_MASK) || (code_buffer [bytes_handled] & RAM_BIT_MASK)) {

            *(prog_buffer + handled_cmds_size) = ' ';
            handled_cmds_size += sizeof (char);
        }

        if (code_buffer [bytes_handled] & RAM_BIT_MASK) {

            *(prog_buffer + handled_cmds_size) = '[';
            handled_cmds_size += sizeof (char);
        }

        if (code_buffer [bytes_handled] & REG_BIT_MASK) {

            sprintf (prog_buffer + handled_cmds_size, "%cx", 'a' + *(code_buffer + bytes_handled + sizeof (unsigned char)));
            handled_cmds_size += 2 * sizeof (char);
            bytes_diff += sizeof (unsigned char);

            if (code_buffer [bytes_handled] & IMM_BIT_MASK) {

                *(prog_buffer + handled_cmds_size) = '+';
                handled_cmds_size += sizeof (char);
            }
        }

        if (code_buffer [bytes_handled] & IMM_BIT_MASK) {

            int num_int = *((int *) (code_buffer + bytes_handled + sizeof (unsigned char) + bytes_diff));
            float num_fl = convert_int_1000_to_float (num_int);
            sprintf (prog_buffer + handled_cmds_size, "%.3f", num_fl);
            handled_cmds_size += sizeof (char) * (get_num_of_digits_int (num_int / 1000) + 4);
            bytes_diff += sizeof (int);
        }

        if (code_buffer [bytes_handled] & RAM_BIT_MASK) {

            *(prog_buffer + handled_cmds_size) = ']';
            handled_cmds_size += sizeof (char);
        }

        *(prog_buffer + handled_cmds_size) = '\n';
        handled_cmds_size += sizeof (char);

        bytes_handled += bytes_diff;
    }

    *disassembled_cmds_size = handled_cmds_size;

    return SUCCESS;
}

void upload_prog (char *prog_buffer, FILE *prog_file, size_t disassembled_cmds_size) {

    assert (prog_buffer);
    assert (prog_file);

    fwrite (prog_buffer, sizeof (char), disassembled_cmds_size, prog_file);
}

void clean_disasm_memory (unsigned char *code_buffer, char *prog_buffer) {

    assert (code_buffer);
    assert (prog_buffer);

    free (code_buffer - sizeof (code_info_t));
    free (prog_buffer);
}