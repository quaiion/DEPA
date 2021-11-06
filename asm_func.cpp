#include "cpu.hpp"

static size_t get_file_size (FILE *file);
static size_t estimate_code_size (char *cmd_index_tbl, int num_of_cmds);
static void set_code_info (unsigned char *code_buffer, code_info_t *code_info);
static int verify_reg_name (int line, char reg);
static int verify_cmd_end_format (int line, char end_symb1, char end_symb2);
static int verify_unknown_cmd (int line, bool cmd_identified);
static int verify_cmd_name_overflow (int line, char *cmd);

FILE *open_code_file (int argc, char *argv []) {

    assert (argv);

    return (argc == 2) ? (fopen ("QO_code_file.bin", "wb")) : (fopen (argv [2], "wb"));
}

size_t get_file_size (FILE *file) {

    assert (file);

    fseek (file, NO_OFFSET, SEEK_END);
    size_t file_size = (size_t) ftell (file);

    rewind (file);
    return file_size;
}

char *store_cmds (FILE *prog_file) {

    assert (prog_file);

    size_t prog_file_size = get_file_size (prog_file);
    
    char *cmd_buffer = (char *) calloc (prog_file_size + 1, sizeof (char));
    if (cmd_buffer == NULL) {

        printf ("\nLOADING FAULT: MEMERY ERROR\n");
        exit (EXIT_FAILURE);
    }

    size_t bytes_read = fread (cmd_buffer, sizeof (char), prog_file_size, prog_file);
    cmd_buffer [bytes_read] = '\0';
    rewind (prog_file);

    return cmd_buffer;
}

int get_num_of_cmds (char *cmd_buffer) {

    assert (cmd_buffer);

    int num_of_cmds = 0;
    char *cmd_end = cmd_buffer - sizeof (char), *cmd_beg = NULL;
    for ( ; cmd_end != NULL; num_of_cmds ++) {

        cmd_beg = cmd_end + sizeof (char);
        cmd_end = strchr (cmd_beg, '\n');
    }

    if (*(cmd_beg) == '\0') {

        num_of_cmds -= 1;
    }

    return num_of_cmds;
}

char **index_cmds (char *cmd_buffer, int num_of_cmds) {

    assert (cmd_buffer);
    assert (num_of_cmds >= 0);

    char **cmd_index_tbl = (char **) calloc (num_of_cmds, sizeof (char *));
    if (cmd_index_tbl == NULL) {

        printf ("\nASSEMBLING FAULT: MEMERY ERROR\n");
        exit (EXIT_FAILURE);
    }

    cmd_index_tbl [0] = cmd_buffer;
    cmd_buffer = strchr (cmd_buffer, '\n');
    for (int i = 1; i < num_of_cmds; i ++) {

        *cmd_buffer = '\0';
        cmd_index_tbl [i] = cmd_buffer + sizeof (char);
        cmd_buffer = strchr (cmd_buffer + sizeof (char), '\n');
    }

    if (cmd_buffer != NULL) {

        *cmd_buffer = '\0';
    }

    return cmd_index_tbl;
}

size_t estimate_code_size (char **cmd_index_tbl, int num_of_cmds) {

    assert (cmd_index_tbl);
    assert (num_of_cmds >= 0);

    int argumented_cmds = 0;

    for (int cmds_handled = 0; cmds_handled < num_of_cmds; cmds_handled ++) {

        char cmd [MAX_CMD_NAME_BYTE_SIZE] = {};

        if (strchr (cmd_index_tbl [cmds_handled], ' ') - cmd_index_tbl [cmds_handled] > MAX_CMD_NAME_BYTE_SIZE ||           \
            (strchr (cmd_index_tbl [cmds_handled], ' ') == NULL && strlen (cmd_index_tbl [cmds_handled]) > MAX_CMD_NAME_BYTE_SIZE)) {

            continue;
        }

        sscanf (cmd_index_tbl [cmds_handled], "%s", cmd);
        if (strcmp (cmd, "push") == STRINGS_EQUAL ||                                    \
            strcmp (cmd, "pop") == STRINGS_EQUAL ||                                     \
            strcmp (cmd, "stvrf") == STRINGS_EQUAL ||                                   \
            strcmp (cmd, "stdmp") == STRINGS_EQUAL) {

            argumented_cmds ++;
        }
    }

    size_t max_code_size = argumented_cmds * (sizeof (int) + sizeof (unsigned char)) + num_of_cmds * sizeof (unsigned char) + sizeof (code_info_t);
    return max_code_size;
}

void set_code_info (unsigned char *code_buffer) {

    assert (code_buffer);

    code_info_t code_info = {};
    code_info.sig = SIGNATURE;
    code_info.arch = 'CISC';

    *((code_info_t *) code_buffer) = code_info;
}

int verify_reg_name (int line, char reg) {

    if (reg < 'a' || reg > 'a' + NUM_OF_REGS - 1) {

        printf ("\nline %d: ASSEMBLING FAULT: WRONG REGISTER NAME\n", line);
        return ERROR;
    }

    return SUCCESS;
}

int verify_cmd_end_format (int line, char end_symb1, char end_symb2) {

    if (end_symb1 != '\0' && !(end_symb1 == ' ' && end_symb2 == '/')) {

        printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", line);
        return ERROR;
    }

    return SUCCESS;
}

int verify_unknown_cmd (int line, bool cmd_identified) {

    if (cmd_identified == false) {

        printf ("\nline %d: ASSEMBLING FAULT: UNKNOWN COMMAND\n", line);
        return ERROR;
    }

    return SUCCESS;
}

int verify_cmd_name_overflow (int line, char *cmd) {

    if (strchr (cmd, ' ') - cmd > MAX_CMD_NAME_BYTE_SIZE ||           \
        (strchr (cmd, ' ') == NULL && strlen (cmd) > MAX_CMD_NAME_BYTE_SIZE)) {

        printf ("\nline %d: ASSEMBLING FAULT: IMPOSSIBLE COMMAND NAME\n", line);
        return ERROR;
    }

    return SUCCESS;
}

int assemble_prog (char **cmd_index_tbl, int num_of_cmds, FILE *code_file) {

    assert (cmd_index_tbl);
    assert (code_file);
    assert (num_of_cmds >= 0);

    size_t max_code_size = estimate_code_size (cmd_index_tbl, num_of_cmds);
    unsigned char *code_buffer = (unsigned char *) calloc (max_code_size, sizeof (unsigned char));
    if (code_buffer == NULL) {

        printf ("\nASSEMBLING FAULT: MEMERY ERROR\n");
        exit (EXIT_FAILURE);
    }

    set_code_info (code_buffer);
    code_buffer += sizeof (code_info_t);

    size_t assembled_code_size = 0;
    for (int cmds_assembled = 0; cmds_assembled < num_of_cmds; cmds_assembled ++) {

        assert (cmd_index_tbl [cmds_assembled]);

        char cmd [MAX_CMD_NAME_BYTE_SIZE] = {};
        char reg = 0, end_symb1 = 0, end_symb2 = 0;
        int arg = 0;
        int format = 0;
        bool cmd_identified = false;

        if (cmd_index_tbl [cmds_assembled] [0] == '/') {

            continue;
        }

        if (verify_cmd_name_overflow (cmds_assembled + 1, cmd_index_tbl [cmds_assembled])) {

            free (code_buffer - sizeof (code_info_t));
            return ERROR;
        }

        sscanf (cmd_index_tbl [cmds_assembled], "%s [%cx+%d]%n%c%c", cmd, &reg, &arg, &format, &end_symb1, &end_symb2);
        if (format) {

            if (verify_cmd_end_format (cmds_assembled + 1, end_symb1, end_symb2) ||            \
                verify_reg_name (cmds_assembled + 1, reg)) {

                free (code_buffer - sizeof (code_info_t));
                return ERROR;
            }

            for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                    if (cmd_code == PUSH || cmd_code == POP) {

                        *(code_buffer + assembled_code_size + sizeof (unsigned char)) = reg - 'a';
                        *((int *) (code_buffer + assembled_code_size + 2 * sizeof (unsigned char))) = arg;

                    } else {

                        printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_assembled + 1);
                        free (code_buffer - sizeof (code_info_t));
                        return ERROR;
                    }

                    cmd_code |= RAM_BIT_MASK;
                    cmd_code |= REG_BIT_MASK;
                    cmd_code |= IMM_BIT_MASK;
                    code_buffer [assembled_code_size ++] = cmd_code;
                    assembled_code_size += sizeof (int) + sizeof (unsigned char);
                    cmd_identified = true;
                    break;
                }
            }
        } else {

            sscanf (cmd_index_tbl [cmds_assembled], "%s %cx+%d%n%c%c", cmd, &reg, &arg, &format, &end_symb1, &end_symb2);
            if (format) {

                if (verify_cmd_end_format (cmds_assembled + 1, end_symb1, end_symb2) ||            \
                    verify_reg_name (cmds_assembled + 1, reg)) {

                    free (code_buffer - sizeof (code_info_t));
                    return ERROR;
                }

                for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                    if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                        if (cmd_code == PUSH) {

                            *(code_buffer + assembled_code_size + sizeof (unsigned char)) = reg - 'a';
                            *((int *) (code_buffer + assembled_code_size + 2 * sizeof (unsigned char))) = arg;

                        } else {

                            printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_assembled + 1);
                            free (code_buffer - sizeof (code_info_t));
                            return ERROR;
                        }

                        cmd_code |= REG_BIT_MASK;
                        cmd_code |= IMM_BIT_MASK;
                        code_buffer [assembled_code_size ++] = cmd_code;
                        assembled_code_size += sizeof (int) + sizeof (unsigned char);
                        cmd_identified = true;
                        break;
                    }
                }
            } else {

                sscanf (cmd_index_tbl [cmds_assembled], "%s [%cx]%n%c%c", cmd, &reg, &format, &end_symb1, &end_symb2);
                if (format) {

                    if (verify_cmd_end_format (cmds_assembled + 1, end_symb1, end_symb2) ||            \
                        verify_reg_name (cmds_assembled + 1, reg)) {

                        free (code_buffer - sizeof (code_info_t));
                        return ERROR;
                    }

                    for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                        if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                            if (cmd_code == PUSH || cmd_code == POP) {

                                *(code_buffer + assembled_code_size + sizeof (unsigned char)) = reg - 'a';

                            } else {

                                printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_assembled + 1);
                                free (code_buffer - sizeof (code_info_t));
                                return ERROR;
                            }

                            cmd_code |= RAM_BIT_MASK;
                            cmd_code |= REG_BIT_MASK;
                            code_buffer [assembled_code_size ++] = cmd_code;
                            assembled_code_size += sizeof (unsigned char);
                            cmd_identified = true;
                            break;
                        }
                    }
                } else {

                    sscanf (cmd_index_tbl [cmds_assembled], "%s [%d]%n%c%c", cmd, &arg, &format, &end_symb1, &end_symb2);
                    if (format) { 

                        if (verify_cmd_end_format (cmds_assembled + 1, end_symb1, end_symb2)) {

                            free (code_buffer - sizeof (code_info_t));
                            return ERROR;
                        }

                        for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                            if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                                if (cmd_code == PUSH || cmd_code == POP) {

                                    *((int *) (code_buffer + assembled_code_size + sizeof (unsigned char))) = arg;

                                } else {

                                    printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_assembled + 1);
                                    free (code_buffer - sizeof (code_info_t));
                                    return ERROR;
                                }

                                cmd_code |= RAM_BIT_MASK;
                                cmd_code |= IMM_BIT_MASK;
                                code_buffer [assembled_code_size ++] = cmd_code;
                                assembled_code_size += sizeof (int);
                                cmd_identified = true;
                                break;
                            }
                        }
                    } else {

                        sscanf (cmd_index_tbl [cmds_assembled], "%s %cx%n%c%c", cmd, &reg, &format, &end_symb1, &end_symb2);
                        if (format) {

                            if (verify_cmd_end_format (cmds_assembled + 1, end_symb1, end_symb2) ||            \
                                verify_reg_name (cmds_assembled + 1, reg)) {

                                free (code_buffer - sizeof (code_info_t));
                                return ERROR;
                            }

                            for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                                if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                                    if (cmd_code == PUSH || cmd_code == POP) {

                                        *(code_buffer + assembled_code_size + sizeof (unsigned char)) = reg - 'a';

                                    } else {

                                        printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_assembled + 1);
                                        free (code_buffer - sizeof (code_info_t));
                                        return ERROR;
                                    }

                                    cmd_code |= REG_BIT_MASK;
                                    code_buffer [assembled_code_size ++] = cmd_code;
                                    assembled_code_size += sizeof (unsigned char);
                                    cmd_identified = true;
                                    break;
                                }
                            }
                        } else {

                            sscanf (cmd_index_tbl [cmds_assembled], "%s %d%n%c%c", cmd, &arg, &format, &end_symb1, &end_symb2);
                            if (format) {

                                if (verify_cmd_end_format (cmds_assembled + 1, end_symb1, end_symb2)) {

                                    free (code_buffer - sizeof (code_info_t));
                                    return ERROR;
                                }

                                for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                                    if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                                        if (cmd_code == PUSH) {

                                            *((int *) (code_buffer + assembled_code_size + sizeof (unsigned char))) = arg;

                                        } else {

                                            printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_assembled + 1);
                                            free (code_buffer - sizeof (code_info_t));
                                            return ERROR;
                                        }

                                        cmd_code |= IMM_BIT_MASK;
                                        code_buffer [assembled_code_size ++] = cmd_code;
                                        assembled_code_size += sizeof (int);
                                        cmd_identified = true;
                                        break;
                                    }
                                }
                            } else {

                                sscanf (cmd_index_tbl [cmds_assembled], "%s%c%c", cmd, &end_symb1, &end_symb2);
                                
                                if (verify_cmd_end_format (cmds_assembled + 1, end_symb1, end_symb2)) {

                                    free (code_buffer - sizeof (code_info_t));
                                    return ERROR;
                                }

                                for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                                    if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                                        code_buffer [assembled_code_size ++] = cmd_code;

                                        if (cmd_code == PUSH) {

                                            printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_assembled + 1);
                                            free (code_buffer - sizeof (code_info_t));
                                            return ERROR;
                                        }

                                        if (cmd_code == STVRF || cmd_code == STDMP) {

                                            *((int *) (code_buffer + assembled_code_size)) = cmds_assembled + 1;
                                            assembled_code_size += sizeof (int);
                                        }

                                        cmd_identified = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (verify_unknown_cmd (cmds_assembled + 1, cmd_identified)) {

            free (code_buffer - sizeof (code_info_t));
            return ERROR;
        }
    }
    
    /*
    printf ("\n");
    for (size_t i = 0; i < assembled_code_size + sizeof (code_info_t); i ++) {

        printf ("%d ", *((unsigned char *) (code_buffer + i - sizeof (code_info_t))));
    }
    printf ("\n");
    */

    code_buffer -= sizeof (code_info_t);
    fwrite (code_buffer, sizeof (unsigned char), assembled_code_size + sizeof (code_info_t), code_file);
    free (code_buffer);

    return SUCCESS;
}

void clean_memory (char **cmd_index_tbl, char *cmd_buffer) {

    assert (cmd_index_tbl);
    assert (cmd_buffer);

    free (cmd_buffer);
    free (cmd_index_tbl);
}