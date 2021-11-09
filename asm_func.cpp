#include "cpu.hpp"

constexpr int format0_const_line_length = 31;
constexpr int format1_const_line_length = 37;
constexpr int format2_const_line_length = 35;
constexpr int format3_const_line_length = 36;
constexpr int format4_const_line_length = 34;
constexpr int format5_const_line_length = 34;
constexpr int format6_const_line_length = 32;
constexpr int format7_const_line_length = 31;

static size_t get_file_size (FILE *file);
static int verify_reg_name (int line, char reg);
static int verify_cmd_end_format (int line, char end_symb1, char end_symb2);
static int verify_unknown_cmd (int line, bool cmd_identified);
static int verify_cmd_name (int line, char *cmd);
static int verify_jump_format (int line, char *cmd);
static void upload_listing (char *listing_buffer, size_t listing_size);
static int get_num_of_digits_int (int number);
static int get_num_of_digits_sizet (size_t number);
static int verify_cmd_space_format (int line, char *cmd);

static size_t get_file_size (FILE *file) {

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

    int low_argumented_cmds = 0;
    int high_argumented_cmds = 0;

    for (int cmds_handled = 0; cmds_handled < num_of_cmds; cmds_handled ++) {

        char cmd [MAX_CMD_NAME_BYTE_SIZE + sizeof (char)] = {};

        sscanf (cmd_index_tbl [cmds_handled], "%s", cmd);
        if (strcmp (cmd, "push") == STRINGS_EQUAL ||                                     \
            strcmp (cmd, "pop") == STRINGS_EQUAL ||                                      \
            strcmp (cmd, "stvrf") == STRINGS_EQUAL ||                                    \
            strcmp (cmd, "stdmp") == STRINGS_EQUAL ||                                    \
            strcmp (cmd, "rgdmp") == STRINGS_EQUAL ||                                    \
            strcmp (cmd, "astdmp") == STRINGS_EQUAL ||                                   \
            strcmp (cmd, "astvrf") == STRINGS_EQUAL) {

            low_argumented_cmds += 1;
        }

        if (strcmp (cmd, "jmp") == STRINGS_EQUAL ||                                      \
            strcmp (cmd, "ja") == STRINGS_EQUAL ||                                       \
            strcmp (cmd, "jae") == STRINGS_EQUAL ||                                      \
            strcmp (cmd, "jb") == STRINGS_EQUAL ||                                       \
            strcmp (cmd, "jbe") == STRINGS_EQUAL ||                                      \
            strcmp (cmd, "je") == STRINGS_EQUAL ||                                       \
            strcmp (cmd, "jne") == STRINGS_EQUAL ||                                      \
            strcmp (cmd, "jf") == STRINGS_EQUAL ||                                       \
            strcmp (cmd, "call") == STRINGS_EQUAL) {

            high_argumented_cmds += 1;
        }
    }

    size_t max_code_size = high_argumented_cmds * sizeof (size_t) + low_argumented_cmds * (sizeof (int) + sizeof (unsigned char))   \
                           + num_of_cmds * sizeof (unsigned char) + sizeof (code_info_t);

    return max_code_size;
}

void set_code_info (unsigned char **code_buffer) {

    assert (code_buffer);

    code_info_t code_info = {};
    code_info.sig = SIGNATURE;
    code_info.arch = 'CISC';

    *((code_info_t *) *code_buffer) = code_info;

    *code_buffer += sizeof (code_info_t);
}

static int verify_reg_name (int line, char reg) {

    if (reg < 'a' || reg > 'a' + NUM_OF_REGS - 1) {

        printf ("\nline %d: ASSEMBLING FAULT: WRONG REGISTER NAME\n", line);
        return ERROR;
    }

    return SUCCESS;
}

static int verify_cmd_end_format (int line, char end_symb1, char end_symb2) {

    if (end_symb1 != '\0' && !(end_symb1 == ' ' && end_symb2 == '/')) {

        printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", line);
        return ERROR;
    }

    return SUCCESS;
}

static int verify_unknown_cmd (int line, bool cmd_identified) {

    if (cmd_identified == false) {

        printf ("\nline %d: ASSEMBLING FAULT: UNKNOWN COMMAND\n", line);
        return ERROR;
    }

    return SUCCESS;
}

static int verify_cmd_name (int line, char *cmd) {

    char *space = strchr (cmd, ' ');

    if (space == NULL) {

        if (strlen (cmd) > MAX_CMD_NAME_BYTE_SIZE) {

            printf ("\nline %d: ASSEMBLING FAULT: IMPOSSIBLE COMMAND NAME (ALLOWED NAME LENGTH EXCEEDED)\n", line);
            return ERROR;
        }

        if (strlen (cmd) < MIN_CMD_NAME_BYTE_SIZE) {

            printf ("\nline %d: ASSEMBLING FAULT: IMPOSSIBLE COMMAND NAME (TOO SHORT NAME)\n", line);
            return ERROR;
        }
    } else {

        if (space - cmd > MAX_CMD_NAME_BYTE_SIZE) {

            printf ("\nline %d: ASSEMBLING FAULT: IMPOSSIBLE COMMAND NAME (ALLOWED NAME LENGTH EXCEEDED)\n", line);
            return ERROR;
        }
        
        if (space - cmd < MIN_CMD_NAME_BYTE_SIZE) {

            printf ("\nline %d: ASSEMBLING FAULT: IMPOSSIBLE COMMAND NAME (TOO SHORT NAME)\n", line);
            return ERROR;
        }
    }

    return SUCCESS;
}

static int verify_cmd_space_format (int line, char *cmd) {

    char *space = strchr (cmd, ' ');

    if (cmd [0] == ' ' ||   \
       (space != NULL && *(space + sizeof (char)) == ' ')) {

        printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", line);
        return ERROR;
    }

    return SUCCESS;
}

static int verify_jump_format (int line, char *cmd) {

    char *space = strchr (cmd, ' ');

    if (space == NULL) {

        printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", line);
        return ERROR;
    }

    if (*(space + sizeof (char)) == '/') {

        printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", line);
        return ERROR;
    }

    char *next_space = strchr (space + sizeof (char), ' ');
    char *line_end = strchr (space + sizeof (char), '\0');

    if (next_space == NULL) {

        if (line_end - (space + sizeof (char)) < MIN_MARK_NAME_BYTE_SIZE) {

            printf ("\nline %d: ASSEMBLING FAULT: IMPOSSIBLE MARK NAME (TOO SHORT NAME)\n", line);
            return ERROR;
        }

        if (line_end - (space + sizeof (char)) > MAX_MARK_NAME_BYTE_SIZE) {

            printf ("\nline %d: ASSEMBLING FAULT: IMPOSSIBLE MARK NAME (ALLOWED NAME LENGTH EXCEEDED)\n", line);
            return ERROR;
        }

    } else {

        if (next_space - (space + sizeof (char)) < MIN_MARK_NAME_BYTE_SIZE) {

            printf ("\nline %d: ASSEMBLING FAULT: IMPOSSIBLE MARK NAME (TOO SHORT NAME)\n", line);
            return ERROR;
        }

        if (next_space - (space + sizeof (char)) > MAX_MARK_NAME_BYTE_SIZE) {

            printf ("\nline %d: ASSEMBLING FAULT: IMPOSSIBLE MARK NAME (ALLOWED NAME LENGTH EXCEEDED)\n", line);
            return ERROR;
        } 
    }

    return SUCCESS;
}

static int verify_unknown_mark (int line, bool mark_identified) {

    if (mark_identified == false) {

        printf ("\nline %d: ASSEMBLING FAULT: UNKNOWN MARK\n", line);
        return ERROR;
    }

    return SUCCESS;
}

int preassemble_prog (char **cmd_index_tbl, int num_of_cmds, mark_t **mark_tbl, int *num_of_marks) {

    assert (cmd_index_tbl);
    assert (num_of_cmds >= 0);

    *mark_tbl = (mark_t *) calloc (DEFAULT_MARK_TABLE_ELEM_SIZE, sizeof (mark_t));
    if (*mark_tbl == NULL) {

        printf ("\nASSEMBLING FAULT: MEMERY ERROR\n");
        exit (EXIT_FAILURE);
    }
    int mark_tbl_size = DEFAULT_MARK_TABLE_ELEM_SIZE;

    size_t handled_code_size = 0;
    int marks_handled = 0;
    for (int cmds_handled = 0; cmds_handled < num_of_cmds; cmds_handled ++) {

        assert (cmd_index_tbl [cmds_handled]);

        char cmd [MAX_CMD_NAME_BYTE_SIZE + sizeof (char)] = {};
        char reg = 0, end_symb1 = 0, end_symb2 = 0;
        int arg = 0, format = 0;
        bool cmd_identified = false;

        if (cmd_index_tbl [cmds_handled] [0] == '/') {

            continue;
        }

        if (verify_cmd_space_format (cmds_handled + 1, cmd_index_tbl [cmds_handled])) {

            return ERROR;
        }

        if (verify_cmd_name (cmds_handled + 1, cmd_index_tbl [cmds_handled])) {

            return ERROR;
        }

        sscanf (cmd_index_tbl [cmds_handled], "%s [%cx+%d]%n%c%c", cmd, &reg, &arg, &format, &end_symb1, &end_symb2);
        if (format) {

            if (verify_cmd_end_format (cmds_handled + 1, end_symb1, end_symb2) ||            \
                verify_reg_name (cmds_handled + 1, reg)) {

                return ERROR;
            }

            for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                    if (cmd_code != PUSH && cmd_code != POP) {

                        printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_handled + 1);
                        return ERROR;
                    }

                    handled_code_size += sizeof (unsigned char) + sizeof (int) + sizeof (unsigned char);
                    cmd_identified = true;
                    break;
                }
            }
        } else {

            sscanf (cmd_index_tbl [cmds_handled], "%s %cx+%d%n%c%c", cmd, &reg, &arg, &format, &end_symb1, &end_symb2);
            if (format) {

                if (verify_cmd_end_format (cmds_handled + 1, end_symb1, end_symb2) ||            \
                    verify_reg_name (cmds_handled + 1, reg)) {

                    return ERROR;
                }

                for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                    if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                        if (cmd_code != PUSH) {

                            printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_handled + 1);
                            return ERROR;
                        }

                        handled_code_size += sizeof (unsigned char) + sizeof (int) + sizeof (unsigned char);
                        cmd_identified = true;
                        break;
                    }
                }
            } else {

                sscanf (cmd_index_tbl [cmds_handled], "%s [%cx]%n%c%c", cmd, &reg, &format, &end_symb1, &end_symb2);
                if (format) {

                    if (verify_cmd_end_format (cmds_handled + 1, end_symb1, end_symb2) ||            \
                        verify_reg_name (cmds_handled + 1, reg)) {

                        return ERROR;
                    }

                    for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                        if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                            if (cmd_code != PUSH && cmd_code != POP) {

                                printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_handled + 1);
                                return ERROR;
                            }

                            handled_code_size += sizeof (unsigned char) + sizeof (unsigned char);
                            cmd_identified = true;
                            break;
                        }
                    }
                } else {

                    sscanf (cmd_index_tbl [cmds_handled], "%s [%d]%n%c%c", cmd, &arg, &format, &end_symb1, &end_symb2);
                    if (format) { 

                        if (verify_cmd_end_format (cmds_handled + 1, end_symb1, end_symb2)) {

                            return ERROR;
                        }

                        for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                            if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                                if (cmd_code != PUSH && cmd_code != POP) {

                                    printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_handled + 1);
                                    return ERROR;
                                }

                                handled_code_size += sizeof (unsigned char) + sizeof (int);
                                cmd_identified = true;
                                break;
                            }
                        }
                    } else {

                        sscanf (cmd_index_tbl [cmds_handled], "%s %cx%n%c%c", cmd, &reg, &format, &end_symb1, &end_symb2);
                        if (format) {

                            if (verify_cmd_end_format (cmds_handled + 1, end_symb1, end_symb2) ||            \
                                verify_reg_name (cmds_handled + 1, reg)) {

                                return ERROR;
                            }

                            for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                                if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                                    if (cmd_code != PUSH && cmd_code != POP) {

                                        printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_handled + 1);
                                        return ERROR;
                                    }

                                    handled_code_size += sizeof (unsigned char) + sizeof (unsigned char);
                                    cmd_identified = true;
                                    break;
                                }
                            }
                        } else {

                            sscanf (cmd_index_tbl [cmds_handled], "%s %d%n%c%c", cmd, &arg, &format, &end_symb1, &end_symb2);
                            if (format) {

                                if (verify_cmd_end_format (cmds_handled + 1, end_symb1, end_symb2)) {

                                    return ERROR;
                                }

                                for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                                    if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                                        if (cmd_code != PUSH) {

                                            printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_handled + 1);
                                            return ERROR;
                                        }

                                        handled_code_size += sizeof (unsigned char) + sizeof (int);
                                        cmd_identified = true;
                                        break;
                                    }
                                }
                            } else {

                                sscanf (cmd_index_tbl [cmds_handled], "%s%c%c", cmd, &end_symb1, &end_symb2);

                                if (cmd [strlen (cmd) - 1] == ':') {

                                    if (verify_cmd_end_format (cmds_handled + 1, end_symb1, end_symb2)) {

                                        return ERROR;
                                    }

                                    if (marks_handled == mark_tbl_size) {

                                        *mark_tbl = (mark_t *) realloc (*mark_tbl, mark_tbl_size * sizeof (mark_t) * 2);
                                        mark_tbl_size *= 2;
                                    }

                                    cmd [strlen (cmd) - 1] = '\0';
                                    strcpy (((*mark_tbl) [marks_handled]).name, cmd);
                                    ((*mark_tbl) [marks_handled]).idx = handled_code_size;
                                    marks_handled += 1;

                                    cmd_index_tbl [cmds_handled] [0] = '/';

                                    continue;
                                }

                                for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                                    if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                                        if (cmd_code == PUSH) {

                                            printf ("\nline %d: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_handled + 1);
                                            return ERROR;
                                        }

                                        handled_code_size += sizeof (unsigned char);

                                        if (cmd_code == STVRF || cmd_code == STDMP || cmd_code == RGDMP ||      \
                                            cmd_code == ASTDMP || cmd_code == ASTVRF) {

                                            handled_code_size += sizeof (int);
                                        }

                                        if (cmd_code == JMP || cmd_code == JA || cmd_code == JAE || cmd_code == JB ||       \
                                            cmd_code == JBE || cmd_code == JE || cmd_code == JNE || cmd_code == JF || cmd_code == CALL) {

                                            char mark_name [MAX_MARK_NAME_BYTE_SIZE + sizeof (char)] = {};

                                            if (verify_jump_format (cmds_handled + 1, cmd_index_tbl [cmds_handled])) {

                                                return ERROR;
                                            } 

                                            end_symb1 = 0, end_symb2 = 0;
                                            sscanf (cmd_index_tbl [cmds_handled], "%s %s%c%c", cmd, mark_name, &end_symb1, &end_symb2);

                                            handled_code_size += sizeof (size_t);
                                        }

                                        if (verify_cmd_end_format (cmds_handled + 1, end_symb1, end_symb2)) {

                                            return ERROR;
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

        if (verify_unknown_cmd (cmds_handled + 1, cmd_identified)) {

            return ERROR;
        }
    }

    *num_of_marks = marks_handled;

    return SUCCESS;
}

int assemble_prog (char **cmd_index_tbl, int num_of_cmds, unsigned char *code_buffer, size_t *assembled_code_size, mark_t *mark_tbl, int num_of_marks) {

    assert (cmd_index_tbl);
    assert (code_buffer);
    assert (num_of_cmds >= 0);
    assert (assembled_code_size);
    assert (num_of_marks >= 0);

    char *listing_buffer = (char *) calloc (num_of_cmds * MAX_LISTING_LINE_SIZE, sizeof (char));
    if (listing_buffer == NULL) {

        printf ("\nASSEMBLING FAULT: MEMERY ERROR\n");
        exit (EXIT_FAILURE);
    }
    size_t symbs_listed = 0;

    size_t handled_code_size = 0;
    for (int cmds_assembled = 0; cmds_assembled < num_of_cmds; cmds_assembled ++) {

        assert (cmd_index_tbl [cmds_assembled]);

        char cmd [MAX_CMD_NAME_BYTE_SIZE + sizeof (char)] = {};
        char reg = 0;
        int arg = 0, format = 0;

        if (cmd_index_tbl [cmds_assembled] [0] == '/') {

            continue;
        }

        sscanf (cmd_index_tbl [cmds_assembled], "%s [%cx+%d]%n", cmd, &reg, &arg, &format);
        if (format) {

            for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                    *(code_buffer + handled_code_size + sizeof (unsigned char)) = reg - 'a';
                    *((int *) (code_buffer + handled_code_size + 2 * sizeof (unsigned char))) = arg;

                    cmd_code |= RAM_BIT_MASK;
                    cmd_code |= REG_BIT_MASK;
                    cmd_code |= IMM_BIT_MASK;
                    sprintf (listing_buffer + symbs_listed, "%.4llX   %.2X   %.2X  %.2X %.2X %.2X %.2X   %s [%cx+%d]\n",   \
                             handled_code_size + sizeof (code_info_t), cmd_code, reg - 'a',   \
                             *(code_buffer + handled_code_size + 2 * sizeof (unsigned char)),   \
                             *(code_buffer + handled_code_size + 3 * sizeof (unsigned char)),   \
                             *(code_buffer + handled_code_size + 4 * sizeof (unsigned char)),   \
                             *(code_buffer + handled_code_size + 5 * sizeof (unsigned char)),   \
                             cmd, reg, arg);
                    symbs_listed += format1_const_line_length + strlen (cmd) + get_num_of_digits_int (arg);
                    code_buffer [handled_code_size ++] = cmd_code;

                    handled_code_size += sizeof (int) + sizeof (unsigned char);
                    break;
                }
            }
        } else {

            sscanf (cmd_index_tbl [cmds_assembled], "%s %cx+%d%n", cmd, &reg, &arg, &format);
            if (format) {

                for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                    if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                        *(code_buffer + handled_code_size + sizeof (unsigned char)) = reg - 'a';
                        *((int *) (code_buffer + handled_code_size + 2 * sizeof (unsigned char))) = arg;

                        cmd_code |= REG_BIT_MASK;
                        cmd_code |= IMM_BIT_MASK;
                        sprintf (listing_buffer + symbs_listed, "%.4llX   %.2X   %.2X  %.2X %.2X %.2X %.2X   %s %cx+%d\n",   \
                                 handled_code_size + sizeof (code_info_t), cmd_code, reg - 'a',   \
                                 *(code_buffer + handled_code_size + 2 * sizeof (unsigned char)),   \
                                 *(code_buffer + handled_code_size + 3 * sizeof (unsigned char)),   \
                                 *(code_buffer + handled_code_size + 4 * sizeof (unsigned char)),   \
                                 *(code_buffer + handled_code_size + 5 * sizeof (unsigned char)),   \
                                 cmd, reg, arg);
                        symbs_listed += format2_const_line_length + strlen (cmd) + get_num_of_digits_int (arg);
                        code_buffer [handled_code_size ++] = cmd_code;
                        handled_code_size += sizeof (int) + sizeof (unsigned char);
                        break;
                    }
                }
            } else {

                sscanf (cmd_index_tbl [cmds_assembled], "%s [%cx]%n", cmd, &reg, &format);
                if (format) {

                    for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                        if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                            *(code_buffer + handled_code_size + sizeof (unsigned char)) = reg - 'a';

                            cmd_code |= RAM_BIT_MASK;
                            cmd_code |= REG_BIT_MASK;
                            sprintf (listing_buffer + symbs_listed, "%.4llX   %.2X   %.2X                %s [%cx]\n",   \
                                     handled_code_size + sizeof (code_info_t), cmd_code, reg - 'a',   \
                                     cmd, reg);
                            symbs_listed += format3_const_line_length + strlen (cmd);
                            code_buffer [handled_code_size ++] = cmd_code;
                            handled_code_size += sizeof (unsigned char);
                            break;
                        }
                    }
                } else {

                    sscanf (cmd_index_tbl [cmds_assembled], "%s [%d]%n", cmd, &arg, &format);
                    if (format) { 

                        for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                            if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                                *((int *) (code_buffer + handled_code_size + sizeof (unsigned char))) = arg;

                                cmd_code |= RAM_BIT_MASK;
                                cmd_code |= IMM_BIT_MASK;
                                sprintf (listing_buffer + symbs_listed, "%.4llX   %.2X       %.2X %.2X %.2X %.2X   %s [%d]\n",   \
                                         handled_code_size + sizeof (code_info_t), cmd_code,   \
                                         *(code_buffer + handled_code_size + sizeof (unsigned char)),       \
                                         *(code_buffer + handled_code_size + 2 * sizeof (unsigned char)),   \
                                         *(code_buffer + handled_code_size + 3 * sizeof (unsigned char)),   \
                                         *(code_buffer + handled_code_size + 4 * sizeof (unsigned char)),   \
                                         cmd, arg);
                                symbs_listed += format4_const_line_length + strlen (cmd) + get_num_of_digits_int (arg);
                                code_buffer [handled_code_size ++] = cmd_code;
                                handled_code_size += sizeof (int);
                                break;
                            }
                        }
                    } else {

                        sscanf (cmd_index_tbl [cmds_assembled], "%s %cx%n", cmd, &reg, &format);
                        if (format) {

                            for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                                if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                                    *(code_buffer + handled_code_size + sizeof (unsigned char)) = reg - 'a';

                                    cmd_code |= REG_BIT_MASK;
                                    sprintf (listing_buffer + symbs_listed, "%.4llX   %.2X   %.2X                %s %cx\n",   \
                                             handled_code_size + sizeof (code_info_t), cmd_code, reg - 'a',   \
                                             cmd, reg);
                                    symbs_listed += format5_const_line_length + strlen (cmd);
                                    code_buffer [handled_code_size ++] = cmd_code;
                                    handled_code_size += sizeof (unsigned char);
                                    break;
                                }
                            }
                        } else {

                            sscanf (cmd_index_tbl [cmds_assembled], "%s %d%n", cmd, &arg, &format);
                            if (format) {

                                for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                                    if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                                        *((int *) (code_buffer + handled_code_size + sizeof (unsigned char))) = arg;

                                        cmd_code |= IMM_BIT_MASK;
                                        sprintf (listing_buffer + symbs_listed, "%.4llX   %.2X       %.2X %.2X %.2X %.2X   %s %d\n",   \
                                                 handled_code_size + sizeof (code_info_t), cmd_code,   \
                                                 *(code_buffer + handled_code_size + sizeof (unsigned char)),       \
                                                 *(code_buffer + handled_code_size + 2 * sizeof (unsigned char)),   \
                                                 *(code_buffer + handled_code_size + 3 * sizeof (unsigned char)),   \
                                                 *(code_buffer + handled_code_size + 4 * sizeof (unsigned char)),   \
                                                 cmd, arg);
                                        symbs_listed += format6_const_line_length + strlen (cmd) + get_num_of_digits_int (arg);
                                        code_buffer [handled_code_size ++] = cmd_code;
                                        handled_code_size += sizeof (int);
                                        break;
                                    }
                                }
                            } else {

                                sscanf (cmd_index_tbl [cmds_assembled], "%s", cmd);

                                for (unsigned char cmd_code = 0; cmd_code < NUM_OF_CMD_TYPES; cmd_code ++) {

                                    if (strcmp (cmd, CMD_NAMES [cmd_code]) == STRINGS_EQUAL) {

                                        code_buffer [handled_code_size ++] = cmd_code;

                                        if (cmd_code == STVRF || cmd_code == STDMP || cmd_code == RGDMP ||      \
                                            cmd_code == ASTDMP || cmd_code == ASTVRF) {

                                            *((int *) (code_buffer + handled_code_size)) = cmds_assembled + 1;

                                            sprintf (listing_buffer + symbs_listed, "%.4llX   %.2X       %.2X %.2X %.2X %.2X   %s\n",   \
                                                     handled_code_size - sizeof (unsigned char) + sizeof (code_info_t), cmd_code,   \
                                                     *(code_buffer + handled_code_size),                                \
                                                     *(code_buffer + handled_code_size + sizeof (unsigned char)),       \
                                                     *(code_buffer + handled_code_size + 2 * sizeof (unsigned char)),   \
                                                     *(code_buffer + handled_code_size + 3 * sizeof (unsigned char)),   \
                                                     cmd);
                                            symbs_listed += format7_const_line_length + strlen (cmd);

                                            handled_code_size += sizeof (int);
                                            
                                        } else if (cmd_code == JMP || cmd_code == JA || cmd_code == JAE || cmd_code == JB ||           \
                                                   cmd_code == JBE || cmd_code == JE || cmd_code == JNE || cmd_code == JF || cmd_code == CALL) {

                                            bool mark_identified = false;
                                            char mark_name [MAX_MARK_NAME_BYTE_SIZE + sizeof (char)] = {};
                                            sscanf (cmd_index_tbl [cmds_assembled], "%s %s", cmd, mark_name);

                                            for (int mark_num = 0; mark_num < num_of_marks; mark_num ++) {

                                                if (strcmp (mark_name, (mark_tbl [mark_num]).name) == STRINGS_EQUAL) {

                                                    *((size_t *) (code_buffer + handled_code_size)) = (mark_tbl [mark_num]).idx;

                                                    sprintf (listing_buffer + symbs_listed, "%.4llX   %.2X       %.4llX          %s %lld\n",   \
                                                             handled_code_size - sizeof (unsigned char) + sizeof (code_info_t), cmd_code,   \
                                                             *((size_t *) (code_buffer + handled_code_size)),       \
                                                             cmd, (mark_tbl [mark_num]).idx);
                                                    symbs_listed += format6_const_line_length + strlen (cmd) + get_num_of_digits_sizet ((mark_tbl [mark_num]).idx);

                                                    handled_code_size += sizeof (size_t);
                                                    mark_identified = true;

                                                    break;
                                                }
                                            }

                                            if (verify_unknown_mark (cmds_assembled + 1, mark_identified)) {

                                                return ERROR;
                                            }
                                        } else {

                                            sprintf (listing_buffer + symbs_listed, "%.4llX   %.2X                     %s\n",   \
                                                     handled_code_size - sizeof (unsigned char) + sizeof (code_info_t), cmd_code,   \
                                                     cmd);
                                            symbs_listed += format0_const_line_length + strlen (cmd);
                                        }

                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    /*
    printf ("\n");
    for (size_t i = 0; i < handled_code_size + sizeof (code_info_t); i ++) {

        printf ("%d ", *((unsigned char *) (code_buffer + i - sizeof (code_info_t))));
    }
    printf ("\n");
    */

    *assembled_code_size = handled_code_size;

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

static int get_num_of_digits_int (int number) {

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

static int get_num_of_digits_sizet (size_t number) {

    int num_of_digits = 0;

    do {

        number /= 10;
        num_of_digits += 1;

    } while (number != 0);

    return num_of_digits;
}

void clean_asm_memory (char **cmd_index_tbl, char *cmd_buffer, unsigned char *code_buffer, mark_t *mark_tbl) {

    assert (cmd_index_tbl);
    assert (cmd_buffer);

    free (cmd_buffer);
    free (cmd_index_tbl);
    if (mark_tbl) {
        
        free (mark_tbl);
    }
    if (code_buffer) {

        free (code_buffer - sizeof (code_info_t));
    }
}