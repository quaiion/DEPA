/* do {} while (0) не используется, потому что объявленные в макросе переменные
не должны быть локальными, а безопасность такой системы гарантируется "закрытым"
кейсом свитча, из которого макросы вызываются
отдельно стоит отметить, что по этой причине все alg-аргументы паттернов должны
в обязательном порядке иметь завершающий символ ";" (если не оканчиваются
закрытием блока)
указать что первые три бита не могут быть заняты типом команды, поэтому
максимум 32 штуки
сказать про функции-верифайеры и про то что можно и без них
энкодер должен проставлять end-символы
про управление через signal (с возможностью немедленного выхода через break) 
в ectrac иметь в виду, что bytes_handled стоит на бацте команды к моменту вызова
и про то, как работает code_ptr (указывает на байт команды)
у листинга маловато знаков в номере байта + слишком много аргументов поломают его нахрен + ограничение на размер записи в строке - по сути он просто колчественно ограничен по все параметрам, но не очень жестко ------ УКАЗАТЬ ВОЗМОЖНОСТЬ РЕДАКТИВРОВАТЬ ЭТИ ПАРАМЕТРЫ */

#define HLT_EXEC signal.stop = true;

#define NO_REG -1
#define NO_IMM 0

#define PUSH_ARG_EXTRAC \
    int imm_arg = NO_IMM; \
    char reg_arg = NO_REG; \
    bool ram_mode = false; \
    int ip_offset = 0; \
    \
    if (*code_ptr & RAM_BIT_MASK) { \
        \
        ram_mode = true; \
    } \
    \
    if (*code_ptr & REG_BIT_MASK) { \
        \
        reg_arg = *((char *) (code_ptr + sizeof (unsigned char))); \
        if (verify_reg_num (reg_arg)) { \
            \
            signal.stop = true; \
            signal.err = true; \
        } \
        ip_offset = sizeof (char); \
        \
        if (*code_ptr & IMM_BIT_MASK) { \
            \
            imm_arg = *((int *) (code_ptr + 2 * sizeof (unsigned char))); \
            ip_offset = += sizeof (int); \
            \
        } \
    } else if (*code_ptr & IMM_BIT_MASK) { \
        \
        imm_arg = *((int *) (code_ptr + sizeof (unsigned char))); \
        ip_offset = sizeof (int); \
        \
    } else { \
        \
        printf ("\nPUSH/POP ARG FORMAT FAULT; EMULATION TERMINATED\n"); \
        signal.stop = true; \
        signal.err = true; \
    }

#define PUSH_EXEC \
    if (ram_mode) { \
        \
        int addr = 0; \
        if (reg_arg != NO_REG) { \
            \
            addr = cpu->reg [reg_arg] / 1000 + imm_arg; \
            \
        } else { \
            \
            addr = imm_arg; \
        } \
            \
        if (verify_segmentation (addr)) { \
            \
            signal.stop = true; \
            signal.err = true; \
            break; \
        } \
        stack_push (stack_p, cpu->ram [addr]); \
        \
    } else { \
        \
        int val = 0; \
        if (reg_arg != NO_REG) { \
            \
            val = cpu->reg [reg_arg] / 1000 + imm_arg; \
            \
        } else { \
            \
            val = imm_arg; \
        } \
        \
        stack_push (stack_p, val); \
    }

#define POP_ARG_EXTRAC \
    int imm_arg = NO_IMM; \
    char reg_arg = NO_REG; \
    bool ram_mode = false; \
    int ip_offset = 0; \
    \
    if (*code_ptr & RAM_BIT_MASK) { \
        \
        ram_mode = true; \
    } \
    \
    if (*code_ptr & REG_BIT_MASK) { \
        \
        reg_arg = *((char *) (code_ptr + sizeof (unsigned char))); \
        if (verify_reg_num (reg)) { \
            \
            signal.stop = true; \
            signal.err = true; \
        } \
        ip_offset = sizeof (char); \
        \
        if (*code_ptr & IMM_BIT_MASK) { \
            \
            if (!ram_mode) { \
                \
                printf ("\nPUSH/POP ARG FORMAT FAULT; EMUALTION TERMINATED\n"); \
                signal.stop = true; \
                signal.err = true; \
                \
            } else { \
                \
                imm_arg = *((int *) (code_ptr + 2 * sizeof (unsigned char))); \
                ip_offset += sizeof (int); \
            } \
        } \
    } else if (*code_ptr & IMM_BIT_MASK) { \
        \
        if (ram_mode) { \
            \
            imm_arg = *((int *) (code_ptr + sizeof (unsigned char))); \
            ip_offset = sizeof (int); \
            \
        } else { \
            \
            printf ("\nPUSH/POP ARG FORMAT FAULT; EMULATION TERMINATED\n"); \
            signal.stop = true; \
            signal.err = true; \
        } \
    } else { \
        \
        if (ram_mode) { \
            \
            printf ("\nPUSH/POP ARG FORMAT FAULT; EMULATION TERMINATED\n"); \
            signal.stop = true; \
            signal.err = true; \
            \
        } \
    }

#define POP_EXEC \
    if (ram_mode) { \
        \
        int addr = 0; \
        if (reg_arg != NO_REG) { \
            \
            addr = cpu->reg [reg_arg] / 1000 + imm_arg; \
            \
        } else { \
            \
            addr = imm_arg; \
        } \
        \
        if (verify_segmentation (addr)) { \
            \
            signal.stop = true; \
            signal.err = true; \
            break; \
        } \
        cpu->ram [addr] = stack_pop (stack_p); \
        \
    } else { \
        \
        int val = 0; \
        if (reg_arg != NO_REG) { \
            \
            cpu->reg [reg_arg] = stack_pop (stack_p); \
            \
        } else { \
            \
            stack_pop (stack_p); \
        } \
    }

#define ADD_EXEC stack_push (stack_p, stack_pop (stack_p) + stack_pop (stack_p));
#define MUL_EXEC stack_push (stack_p, stack_pop (stack_p) * stack_pop (stack_p) / 1000);

#define SUB_EXEC \
    int temp_val = stack_pop (stack_p); \
    stack_push (stack_p, stack_pop (stack_p) - temp_val);

#define DIV_EXEC \
    int temp_val = stack_pop (stack_p); \
    stack_push (stack_p, (stack_pop (stack_p) * 1000) / temp_val);

#define IN_EXEC \
    float num_fl = 0; \
    scanf ("%f", &num_fl); \
    if (verify_in_num (num_fl)) { \
        \
        signal.stop = true; \
        signal.err = true; \
    } \
    \
    int num_int = convert_float_to_int_1000 (num_fl); \
    stack_push (stack_p, num_int);

#define OUT_EXEC \
    float num_fl = convert_int_1000_to_float (stack_pop (stack_p)); \
    printf ("%.3f\n", num_fl);

#define CODE_LINE_NUM_EXTRAC \
    unsigned long code_line = *((unsigned long *) (code_ptr + sizeof (unsigned char)));

#define STDMP_EXEC stack_dump (stack_p, code_line);
#define ASTDMP_EXEC addr_stack_dump (addr_stack_p, code_line);
#define STVRF_EXEC stack_verify (stack_p, code_line);
#define ASTVRF_EXEC addr_stack_verify (addr_stack_p, code_line);
#define RGDMP_EXEC reg_dump (cpu, code_line);

#define BYTE_POS_EXTRAC \
    size_t byte_pos = *((size_t *) (code_ptr + sizeof (unsigned char))) - sizeof (unsigned char);

#define JMP_EXEC cpu->ip = byte_pos - arg_byte_size;

#define JF_EXEC \
    time_t tim = time (NULL); \
    if (localtime (&tim)->tm_wday == FRIDAY) { \
        \
        cpu->ip = byte_pos - arg_byte_size; \
    }

#define COMMON_COND_JMP_EXEC(cond) \
    int temp_val_2 = stack_pop (stack_p), temp_val_1 = stack_pop (stack_p); \
    stack_push (stack_p, temp_val_1); \
    stack_push (stack_p, temp_val_2); \
    \
    if (cond) { \
        \
        cpu->ip = byte_pos - arg_byte_size; \
    }

#define JA_COND temp_val_1 > temp_val_2
#define JAE_COND temp_val_1 >= temp_val_2
#define JB_COND temp_val_1 < temp_val_2
#define JBE_COND temp_val_1 <= temp_val_2
#define JE_COND temp_val_1 == temp_val_2
#define JNE_COND temp_val_1 != temp_val_2

#define CALL_EXEC \
    addr_stack_push (addr_stack_p, cpu->ip + sizeof (size_t)); \
    cpu->ip = byte_pos - arg_byte_size;

#define RET_EXEC cpu->ip = addr_stack_pop (addr_stack_p);

#define IRR_PREASM \
    int ip_offset = 0; \
    if (strchr (space, '[')) { \
        \
        cmd_index_tbl [cmds_handled]->cmd_cnst |= RAM_BIT_MASK; \
    } \
    if (strchr (space, 'x')) { \
        \
        cmd_index_tbl [cmds_handled]->cmd_cnst |= REG_BIT_MASK; \
        ip_offset = sizeof (unsigned char); \
        if (strchr (space, '+')) { \
            \
            cmd_index_tbl [cmds_handled]->cmd_cnst |= IMM_BIT_MASK; \
            ip_offset += sizeof (int); \
        } \
    } else if (search_digit (space))) { \
        \
        cmd_index_tbl [cmds_handled]->cmd_cnst |= IMM_BIT_MASK; \
        ip_offset = sizeof (int); \
    }

#define PUSH_ARG_ASM \
    int ip_offset = 0; \
    if (cmd_cnst & RAM_BIT_MASK) { \
        \
        if (cmd_cnst & REG_BIT_MASK) { \
            \
            ip_offset = sizeof (unsigned char); \
            \
            if (cmd_cnst & IMM_BIT_MASK) { \
                \
                sscanf (space + 1, "[%cx+%d]%n%c%c", %reg, &arg, &format, &end_symb_1, &end_symb_2); \
                if (verify_cmd_arg_format (cmds_assembled + 1, format) || \
                    verify_reg_name (cmds_assembled + 1, reg)) { \
                    \
                    signal.stop = true; \
                    signal.err = true; \
                    break; \
                } \
                \
                *(code_buffer + handled_code_size) = reg - 'a'; \
                *((int *) (code_buffer + handled_code_size + sizeof (unsigned char))) = arg; \
                ip_offset += sizeof (int); \
                \
            } else { \
                \
                sscanf (space + 1, "[%cx]%n%c%c", &reg, &format, &end_symb_1, &end_symb_2); \
                if (verify_cmd_arg_format (cmds_assembled + 1, format) || \
                    verify_reg_name (cmds_assembled + 1, reg)) { \
                    \
                    signal.stop = true; \
                    signal.err = true; \
                    break; \
                } \
                \
                *(code_buffer + handled_code_size) = reg - 'a'; \
            } \
        } else if (cmd_cnst & IMM_BIT_MASK) { \
            \
            sscanf (space + 1, "[%d]%n%c%c", &arg, &format, &end_symb_1, &end_symb_2); \
            if (verify_cmd_arg_format (cmds_assembled + 1, format)) { \
                \
                signal.stop = true; \
                signal.err = true; \
                break; \
            } \
            \
            *((int *) (code_buffer + handled_code_size)) = arg; \
            ip_offset = sizeof (int); \
            \
        } else { /* Этот кусочек кода можно было сократить, подцепив к последнему else, но так нагляднее */ \
            \
            printf ("\nline %lu: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_assembled + 1); \
            signal.stop = true; \
            signal.err = true; \
            break; \
        } \
    } else { \
        \
        if (cmd_cnst & REG_BIT_MASK) { \
            \
            ip_offset = sizeof (unsigned char); \
            \
            if (cmd_cnst & IMM_BIT_MASK) { \
                \
                sscanf (space + 1, "%cx+%f%n%c%c", &reg, &fl_num, &format, &end_symb_1, &end_symb_2); \
                if (verify_cmd_arg_format (cmds_assembled + 1, format) || \
                    verify_reg_name (cmds_assembled + 1, reg) || \
                    verify_arg_num (cmds_assembled + 1, fl_num)) { \
                    \
                    signal.stop = true; \
                    signal.err = true; \
                    break; \
                } \
                \
                int num_int = convert_float_to_int_1000 (num_fl); \
                *(code_buffer + handled_code_size) = reg - 'a'; \
                *((int *) (code_buffer + handled_code_size + sizeof (unsigned char))) = int_num; \
                ip_offset += sizeof (int); \
                \
            } else { \
                \
                sscanf (space + 1, "%cx%n%c%c", &reg, &format, &end_symb_1, &end_symb_2); \
                if (verify_cmd_arg_format (cmds_assembled + 1, format) || \
                    verify_reg_name (cmds_assembled + 1, reg)) { \
                    \
                    signal.stop = true; \
                    signal.err = true; \
                    break; \
                } \
                \
                *(code_buffer + handled_code_size) = reg - 'a'; \
                \
            } \
        } else if (cmd_cnst & IMM_BIT_MASK) { \
            \
            sscanf (space + 1, "%f%n%c%c", &fl_num, &format, &end_symb_1, &end_symb_2); \
            if (verify_cmd_arg_format (cmds_assembled + 1, format) || \
                verify_arg_num (cmds_assembled + 1, fl_num)) { \
                \
                signal.stop = true; \
                signal.err = true; \
                break; \
            } \
            \
            int num_int = convert_float_to_int_1000 (num_fl); \
            *((int *) (code_buffer + handled_code_size)) = num_int; \
            ip_offset = sizeof (int); \
            \
        } else {    /* Этот кусочек кода тоже */ \
            \
            printf ("\nline %lu: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_assembled + 1); \
            signal.stop = true; \
            signal.err = true; \
            break; \
        } \
    }

#define POP_ARG_ASM \
    bool ext_arg = true; \
    int ip_offset = 0; \
    \
    if (cmd_cnst & RAM_BIT_MASK) { \
        \
        if (cmd_cnst & REG_BIT_MASK) { \
            \
            ip_offset = sizeof (unsigned char); \
            \
            if (cmd_cnst & IMM_BIT_MASK) { \
                \
                sscanf (space + 1, "[%cx+%d]%n%c%c", %reg, &arg, &format, &end_symb_1, &end_symb_2); \
                if (verify_cmd_arg_format (cmds_assembled + 1, format) || \
                    verify_reg_name (cmds_assembled + 1, reg)) { \
                    \
                    signal.stop = true; \
                    signal.err = true; \
                    break; \
                } \
                \
                *(code_buffer + handled_code_size) = reg - 'a'; \
                *((int *) (code_buffer + handled_code_size + sizeof (unsigned char))) = arg; \
                ip_offset += sizeof (int); \
                \
            } else { \
                \
                sscanf (space + 1, "[%cx]%n%c%c", &reg, &format, &end_symb_1, &end_symb_2); \
                if (verify_cmd_arg_format (cmds_assembled + 1, format) || \
                    verify_reg_name (cmds_assembled + 1, reg)) { \
                    \
                    signal.stop = true; \
                    signal.err = true; \
                    break; \
                } \
                \
                *(code_buffer + handled_code_size) = reg - 'a'; \
            } \
        } else if (cmd_cnst & IMM_BIT_MASK) { \
            \
            sscanf (space + 1, "[%d]%n%c%c", &arg, &format, &end_symb_1, &end_symb_2); \
            if (verify_cmd_arg_format (cmds_assembled + 1, format)) { \
                \
                signal.stop = true; \
                signal.err = true; \
                break; \
            } \
            \
            *((int *) (code_buffer + handled_code_size)) = arg; \
            ip_offset = sizeof (int); \
            \
        } else {    /* И этот кусочек */ \
            \
            printf ("\nline %lu: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_assembled + 1); \
            signal.stop = true; \
            signal.err = true; \
            break; \
        } \
    } else { \
        \
        if (cmd_cnst & IMM_BIT_MASK) { \
            \
            printf ("\nline %lu: ASSEMBLING FAULT: WRONG FORMAT\n", cmds_assembled + 1); \
            signal.stop = true; \
            signal.err = true; \
            break; \
        } \
        \
        if (cmd_cnst & REG_BIT_MASK) { \
            \
            ip_offset = sizeof (unsigned char); \
            \
            sscanf (space + 1, "%cx%n%c%c", &reg, &format, &end_symb_1, &end_symb_2); \
            if (verify_cmd_arg_format (cmds_assembled + 1, format) || \
                verify_reg_name (cmds_assembled + 1, reg)) { \
                \
                signal.stop = true; \
                signal.err = true; \
                break; \
            } \
            \
            *(code_buffer + handled_code_size) = reg - 'a'; \
            \
        } else { \
            \
            ext_arg = false; \
        } \
    }

#define DMP_VRF_ARG_ASM *((unsigned long *) (code_buffer + handled_code_size)) = cmds_assembled + 1;

#define JMP_ARG_ASM \
    if (verify_jump_format (cmds_assembled + 1, cmd_index_tbl + cmds_assembled)) { \
        \
        signal.stop = true; \
        signal.err = true; \
        break; \
    } \
    \
    char mark [MAX_MARK_NAME_BYTE_SIZE + 1] = {}; \
    sscanf (space + 1, "%s%c%c", mark, &end_symb_1, &end_symb_2); \
    int mark_num = lin_search_mrk (mark, mark_tbl, num_of_marks); \
    if (verify_unknown_mark (cmds_assembled + 1, mark_num)) { \
        \
        signal.stop = true; \
        signal.err = true; \
        break; \
    } \
    *((size_t *) (code_buffer + handled_code_size)) = mark_tbl [mark_num].idx;

#define PUSH_ARG_DIS_PRINT \
    int symbs = 0; \
    \
    if (ram_mode) { \
        \
        *arg_str = '['; \
        symbs += sizeof (char); \
    } \
    if (reg_arg != NO_REG) { \
        \
        sprintf (arg_str + symbs, "%cx", reg_arg + 'a'); \
        symbs += 2 * sizeof (char); \
        if (imm_arg != NO_IMM) { \
            \
            *(arg_str + symbs) = '+'; \
            symbs += sizeof (char); \
        } \
    } \
    if (imm_arg != NO_IMM) { \
        \
        if (ram_mode) { \
            \
            sprintf (arg_str + symbs, "%d]", imm_arg); \
            \
        } else { \
            \
            float fl_num = convert_int_1000_to_float (imm_arg); \
            sprintf (arg_str + symbs, "%.3f", fl_num); \
        } \
    }

#define POP_ARG_DIS_PRINT \
    int symbs = 0; \
    \
    if (ram_mode) { \
        \
        *arg_str = '['; \
        symbs += sizeof (char); \
    } \
    if (reg_arg != NO_REG) { \
        \
        sprintf (arg_str + symbs, "%cx", reg_arg + 'a'); \
        symbs += 2 * sizeof (char); \
        if (imm_arg != NO_IMM) { \
            \
            *(arg_str + symbs) = '+'; \
            symbs += sizeof (char); \
        } \
    } \
    if (imm_arg != NO_IMM) {        /* вариант с НЕ ram_mode запрещен и отсеивается на стадии extract */ \
        \
        sprintf (arg_str + symbs, "%d]", imm_arg); \
    }

#define BYTE_POS_DIS_PRINT sprintf (arg_str, "%llu", byte_pos);

#define PRNT_EXEC \
    printf ("\n\n"); \
    for (int i = 0; i < CONSOLE_SIZE_VERT; ++i) { \
        \
        int subseg = VIDEOSEG_START + i * CONSOLE_SIZE_HOR; \
        printf ("\n"); \
        for (int j = 0; j < CONSOLE_SIZE_HOR; ++j) { \
            \
            if (cpu->ram [subseg + j]) { \
                \
                printf ("*"); \
                \
            } else { \
                \
                printf (" "); \
            } \
        } \
    } \

#define SHW_EXEC \
    printf ("\n\n"); \
    for (int i = 0; i < CONSOLE_SIZE_VERT; ++i) { \
        \
        int subseg = VIDEOSEG_START + i * CONSOLE_SIZE_HOR; \
        printf ("\n"); \
        for (int j = 0; j < CONSOLE_SIZE_HOR; ++j) { \
            \
            if (cpu->ram [subseg + j]) { \
                \
                printf ("*"); \
                \
            } else { \
                \
                printf (" "); \
            } \
        } \
    } \
    for (int i = 0; i < CONSOLE_SIZE_VERT; ++i) { \
        \
        printf ("\n"); \
    }


/* CMD_PATTERN(name_cnst, token, arg_extraction_alg, arg_byte_size, execution_alg, preasm_format_alg, extern_arg, arg_assem_alg, arg_disas_print, max_disasm_arg_len) */
CMD_PATTERN (HLT, hlt, /* no_arg */, 0, HLT_EXEC, /* no_spec_instr */, false, /* no_arg */, /* no_arg */, 0)
CMD_PATTERN (PUSH, push, PUSH_ARG_EXTRAC, ip_offset, PUSH_EXEC, IRR_PREASM, true, PUSH_ARG_ASM, PUSH_ARG_DIS_PRINT, 24)
CMD_PATTERN (POP, pop, POP_ARG_EXTRAC, ip_offset, POP_EXEC, IRR_PREASM, ext_arg, POP_ARG_ASM, POP_ARG_DIS_PRINT, 24)
CMD_PATTERN (ADD, add, /* no_arg */, 0, ADD_EXEC, /* no_spec_instr */, false, /* no_arg */, /* no_arg */, 0)
CMD_PATTERN (SUB, sub, /* no_arg */, 0, SUB_EXEC, /* no_spec_instr */, false, /* no_arg */, /* no_arg */, 0)
CMD_PATTERN (MUL, mul, /* no_arg */, 0, MUL_EXEC, /* no_spec_instr */, false, /* no_arg */, /* no_arg */, 0)
CMD_PATTERN (DIV, div, /* no_arg */, 0, DIV_EXEC, /* no_spec_instr */, false, /* no_arg */, /* no_arg */, 0)
CMD_PATTERN (IN, in, /* no_arg */, 0, IN_EXEC, /* no_spec_instr */, false, /* no_arg */, /* no_arg */, 0)
CMD_PATTERN (OUT, out, /* no_arg */, 0, OUT_EXEC, /* no_spec_instr */, false, /* no_arg */, /* no_arg */, 0)
CMD_PATTERN (STDMP, stdmp, CODE_LINE_NUM_EXTRAC, sizeof (unsigned long), STDMP_EXEC, /* no_spec_instr */, false, DMP_VRF_ARG_ASM, /* no_arg_printed */, 0)
CMD_PATTERN (STVRF, stvrf, CODE_LINE_NUM_EXTRAC, sizeof (unsigned long), STVRF_EXEC, /* no_spec_instr */, false, DMP_VRF_ARG_ASM, /* no_arg_printed */, 0)
CMD_PATTERN (ASTDMP, astdmp, CODE_LINE_NUM_EXTRAC, sizeof (unsigned long), ASTDMP_EXEC, /* no_spec_instr */, false, DMP_VRF_ARG_ASM, /* no_arg_printed */, 0)
CMD_PATTERN (ASTVRF, astvrf, CODE_LINE_NUM_EXTRAC, sizeof (unsigned long), ASTVRF_EXEC, /* no_spec_instr */, false, DMP_VRF_ARG_ASM, /* no_arg_printed */, 0)
CMD_PATTERN (RGDMP, rgdmp, CODE_LINE_NUM_EXTRAC, sizeof (unsigned long), RGDMP_EXEC, /* no_spec_instr */, false, DMP_VRF_ARG_ASM, /* no_arg_printed */, 0)
CMD_PATTERN (JMP, jmp, BYTE_POS_EXTRAC, sizeof (size_t), JMP_EXEC, /* no_spec_instr */, true, JMP_ARG_ASM, BYTE_POS_DIS_PRINT, 24)
CMD_PATTERN (JA, ja, BYTE_POS_EXTRAC, sizeof (size_t), COMMON_COND_JMP_EXEC (JA_COND), /* no_spec_instr */, true, JMP_ARG_ASM, BYTE_POS_DIS_PRINT, 24)
CMD_PATTERN (JAE, jae, BYTE_POS_EXTRAC, sizeof (size_t), COMMON_COND_JMP_EXEC (JAE_COND), /* no_spec_instr */, true, JMP_ARG_ASM, BYTE_POS_DIS_PRINT, 24)
CMD_PATTERN (JB, jb, BYTE_POS_EXTRAC, sizeof (size_t), COMMON_COND_JMP_EXEC (JB_COND), /* no_spec_instr */, true, JMP_ARG_ASM, BYTE_POS_DIS_PRINT, 24)
CMD_PATTERN (JBE, jbe, BYTE_POS_EXTRAC, sizeof (size_t), COMMON_COND_JMP_EXEC (JBE_COND), /* no_spec_instr */, true, JMP_ARG_ASM, BYTE_POS_DIS_PRINT, 24)
CMD_PATTERN (JE, je, BYTE_POS_EXTRAC, sizeof (size_t), COMMON_COND_JMP_EXEC (JE_COND), /* no_spec_instr */, true, JMP_ARG_ASM, BYTE_POS_DIS_PRINT, 24)
CMD_PATTERN (JNE, jne, BYTE_POS_EXTRAC, sizeof (size_t), COMMON_COND_JMP_EXEC (JNE_COND), /* no_spec_instr */, true, JMP_ARG_ASM, BYTE_POS_DIS_PRINT, 24)
CMD_PATTERN (JF, jf, BYTE_POS_EXTRAC, sizeof (size_t), JF_EXEC, /* no_spec_instr */, true, JMP_ARG_ASM, BYTE_POS_DIS_PRINT, 24)
CMD_PATTERN (CALL, call, BYTE_POS_EXTRAC, sizeof (size_t), CALL_EXEC, /* no_spec_instr */, true, JMP_ARG_ASM, BYTE_POS_DIS_PRINT, 24)
CMD_PATTERN (RET, ret, /* no_arg */, 0, RET_EXEC, /* no_spec_instr */, false, /* no_arg */, /* no_arg */, 0)
CMD_PATTERN (PRNT, prnt, /* no_arg */, 0, PRNT_EXEC, /* no_spec_instr */, false, /* no_arg */, /* no_arg */, 0)
CMD_PATTERN (SHW, shw, /* no_arg */, 0, SHW_EXEC, /* no_spec_instr */, false, /* no_arg */, /* no_arg */, 0)


#undef HLT_EXEC
#undef PUSH_ARG_EXTRAC
#undef PUSH_EXEC
#undef POP_ARG_EXTRAC
#undef POP_EXEC
#undef ADD_EXEC
#undef MUL_EXEC
#undef SUB_EXEC
#undef DIV_EXEC
#undef IN_EXEC
#undef OUT_EXEC
#undef CODE_LINE_NUM_EXTRAC
#undef STDMP_EXEC
#undef ASTDMP_EXEC
#undef STVRF_EXEC
#undef ASTVRF_EXEC
#undef RGDMP_EXEC
#undef BYTE_POS_EXTRAC
#undef JMP_EXEC
#undef JF_EXEC
#undef COMMON_COND_JMP_EXEC
#undef JA_COND
#undef JAE_COND
#undef JB_COND
#undef JBE_COND
#undef JE_COND
#undef JNE_COND
#undef CALL_EXEC
#undef RET_EXEC
#undef IRR_PREASM
#undef PUSH_ARG_ASM
#undef POP_ARG_ASM
#undef DMP_VRF_ARG_ASM
#undef JMP_ARG_ASM  /* классное название макроса получилось */
#undef PUSH_ARG_DIS_PRINT
#undef POP_ARG_DIS_PRINT
#undef BYTE_POS_DIS_PRINT
#undef NO_REG
#undef NO_IMM
#undef PRNT_EXEC
#undef SHW_EXEC
