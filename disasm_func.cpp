#include "cpu.hpp"

static size_t estimate_prog_size (unsigned char *code_buffer, size_t code_buffer_size);
static size_t get_file_size (FILE *file);
static int get_num_of_digits (int number);
static int verify_code_architecture (unsigned char *code_buffer);
static int verify_cmd_code (size_t byte_number, unsigned char cmd_code);

FILE *open_prog_file (int argc, char *argv []) {

    assert (argv);

    return (argc == 2) ? (fopen ("QO_prog_file.asm", "w")) : (fopen (argv [2], "w"));
}

size_t get_file_size (FILE *file) {

    assert (file);

    fseek (file, NO_OFFSET, SEEK_END);
    size_t file_size = (size_t) ftell (file);

    rewind (file);
    return file_size;
}

int verify_code_architecture (unsigned char *code_buffer) {

    assert (code_buffer);

    if ((*((code_info_t *) code_buffer)).arch != 'CISC') {

        printf ("\nDISASSEMBLING FAULT: WRONG CODE ARCHITECTURE\n");
        return ERROR;
    }

    return SUCCESS;
}

int verify_cmd_code (size_t byte_number, unsigned char cmd_code) {

    if ((cmd_code & ONLY_CMD_TYPE_MASK) > NUM_OF_CMD_TYPES - 1) {

        printf ("\nbyte %llu: DISASSEMBLING FAULT: UNKNOWN COMMAND CODE\n", byte_number);
        return ERROR;
    }

    return SUCCESS;
}

int get_num_of_digits (int number) {

    int num_of_digits = 0;

    if (number < 0) {

        num_of_digits = 1;
    }

    do {

        number /= 10;
        num_of_digits += 1;

    } while (number != 0);

    return num_of_digits;
}

unsigned char *store_code (FILE *code_file, size_t *code_buffer_size) {

    assert (code_buffer_size);
    assert (code_file);

    size_t code_file_size = get_file_size (code_file);
    
    unsigned char *code_buffer = (unsigned char *) calloc (code_file_size, sizeof (unsigned char));
    if (code_buffer == NULL) {

        printf ("\nLOADING FAULT: MEMERY ERROR\n");
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

        size_t bytes_diff = 0;
        max_cmds_size += MAX_CMD_NAME_BYTE_SIZE + 3 * sizeof (char);

        if (code_buffer [bytes_handled] == STVRF || code_buffer [bytes_handled] == STDMP) {

            bytes_handled += sizeof (int);
            continue;
        }

        if (code_buffer [bytes_handled] & RAM_BIT_MASK) {

            max_cmds_size += 2 * sizeof (char);
        }

        if (code_buffer [bytes_handled] & REG_BIT_MASK) {

            max_cmds_size += 2 * sizeof (char);
            bytes_diff += sizeof (unsigned char);
        }

        if (code_buffer [bytes_handled] & IMM_BIT_MASK) {

            max_cmds_size += sizeof (char) * get_num_of_digits (*((int *) (code_buffer + bytes_handled + sizeof (unsigned char) + bytes_diff)));
            bytes_diff += sizeof (int);
        }

        bytes_handled += bytes_diff;
    }

    return max_cmds_size;
}

int disassemble_code (unsigned char* code_buffer, size_t code_buffer_size, FILE *prog_file) {

    assert (code_buffer);
    assert (prog_file);

    if (verify_code_architecture (code_buffer - sizeof (code_info_t))) {

        return ERROR;
    }

    /*
    printf ("\n");
    for (size_t i = 0; i < code_buffer_size + sizeof (code_info_t); i ++) {

        printf ("%d ", *((unsigned char *) (code_buffer + i - sizeof (code_info_t))));
    }
    printf ("\n");
    */

    size_t max_prog_size = estimate_prog_size (code_buffer, code_buffer_size);
    char *prog_buffer = (char *) calloc (max_prog_size, sizeof (char));
    if (prog_buffer == NULL) {

        printf ("DISASSEMBLING FAULT: MEMERY ERROR");
        exit (EXIT_FAILURE);
    }

    size_t disassembled_cmds_size = 0;
    for (size_t bytes_handled = 0; bytes_handled < code_buffer_size; bytes_handled ++) {

        int arg = 0;
        size_t bytes_diff = 0;

        if (verify_cmd_code (bytes_handled + sizeof (code_info_t), code_buffer [bytes_handled])) {

            free (prog_buffer);
            return ERROR;
        }
        sprintf (prog_buffer + disassembled_cmds_size, "%s", CMD_NAMES [code_buffer [bytes_handled] & ONLY_CMD_TYPE_MASK]);
        disassembled_cmds_size += strlen (CMD_NAMES [code_buffer [bytes_handled] & ONLY_CMD_TYPE_MASK]) * sizeof (char);

        if (code_buffer [bytes_handled] == STVRF || code_buffer [bytes_handled] == STDMP) {

            *(prog_buffer + disassembled_cmds_size) = '\n';

            disassembled_cmds_size += (sizeof (char));
            bytes_handled += sizeof (int);
            continue;
        }

        if ((code_buffer [bytes_handled] & REG_BIT_MASK) || (code_buffer [bytes_handled] & IMM_BIT_MASK) || (code_buffer [bytes_handled] & RAM_BIT_MASK)) {

            *(prog_buffer + disassembled_cmds_size) = ' ';
            disassembled_cmds_size += sizeof (char);
        }

        if (code_buffer [bytes_handled] & RAM_BIT_MASK) {

            *(prog_buffer + disassembled_cmds_size) = '[';
            disassembled_cmds_size += sizeof (char);
        }

        if (code_buffer [bytes_handled] & REG_BIT_MASK) {

            sprintf (prog_buffer + disassembled_cmds_size, "%cx", 'a' + *(code_buffer + bytes_handled + sizeof (unsigned char)));
            disassembled_cmds_size += 2 * sizeof (char);
            bytes_diff += sizeof (unsigned char);

            if (code_buffer [bytes_handled] & IMM_BIT_MASK) {

                *(prog_buffer + disassembled_cmds_size) = '+';
                disassembled_cmds_size += sizeof (char);
            }
        }

        if (code_buffer [bytes_handled] & IMM_BIT_MASK) {

            arg = *((int *) (code_buffer + bytes_handled + sizeof (unsigned char) + bytes_diff));
            sprintf (prog_buffer + disassembled_cmds_size, "%d", arg);
            disassembled_cmds_size += sizeof (char) * get_num_of_digits (arg);
            bytes_diff += sizeof (int);
        }

        if (code_buffer [bytes_handled] & RAM_BIT_MASK) {

            *(prog_buffer + disassembled_cmds_size) = ']';
            disassembled_cmds_size += sizeof (char);
        }

        *(prog_buffer + disassembled_cmds_size) = '\n';
        disassembled_cmds_size += sizeof (char);

        bytes_handled += bytes_diff;
    }

    fwrite (prog_buffer, sizeof (char), disassembled_cmds_size, prog_file);
    free (prog_buffer);

    return SUCCESS;
}