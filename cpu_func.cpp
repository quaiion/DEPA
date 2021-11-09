#include "cpu.hpp"
#include "stack_func.cpp"

static size_t get_file_size (FILE *file);
static int verify_code_architecture (unsigned char *code_buffer);
static void clean_buffer_memory (unsigned char *code_buffer);
static void reg_dump (cpu_t *cpu, int line);

void cpu_ctor (cpu_t *cpu) {

    assert (cpu);

    cpu->arch = 'CISC';

    cpu->ram = (int *) calloc (RAM_SIZE, sizeof (int));
    if (cpu->ram == NULL) {
        
        printf ("CONSTRUCTION FAULT: MEMORY ERROR");
        exit (EXIT_FAILURE);
    }

    cpu->ip = 0;
    cpu->code = NULL;
    cpu->code_size = 0;

    for (int i = 0; i < NUM_OF_REGS; i ++) {

        cpu->reg [i] = 0;
    }

    stack_ctor (&(cpu->stack));
    addr_stack_ctor (&(cpu->addr_stack));
}

void cpu_dtor (cpu_t *cpu) {

    assert (cpu);

    cpu->arch = 'NOPE';

    free (cpu->ram);
    cpu->ram = NULL;

    cpu->ip = 0;
    clean_buffer_memory (cpu->code);
    cpu->code = NULL;
    cpu->code_size = 0;

    for (int i = 0; i < NUM_OF_REGS; i ++) {

        cpu->reg [i] = 0;
    }

    stack_dtor (&(cpu->stack));
    addr_stack_dtor (&(cpu->addr_stack));
}

static size_t get_file_size (FILE *file) {

    assert (file);

    fseek (file, NO_OFFSET, SEEK_END);
    size_t file_size = (size_t) ftell (file);

    rewind (file);
    return file_size;
}

static int verify_code_architecture (unsigned char *code_buffer) {

    assert (code_buffer);

    if ((*((code_info_t *) code_buffer)).arch != 'CISC') {

        if ((*((code_info_t *) code_buffer)).arch == 'NOPE') {

            printf ("\nEXECUTION FAULT: CPU EMULATION IS DISTRUCTED\n");
            return ERROR;
        }

        printf ("\nEXECUTION FAULT: WRONG CODE ARCHITECTURE\n");
        return ERROR;
    }

    return SUCCESS;
}

void load_code (FILE *code_file, cpu_t *cpu) {

    assert (code_file);
    assert (cpu);

    size_t code_file_size = get_file_size (code_file);

    cpu->code = (unsigned char *) calloc (code_file_size, sizeof (unsigned char));
    if (cpu->code == NULL) {

        printf ("\nLOADING FAULT: MEMERY ERROR\n");
        exit (EXIT_FAILURE);
    }

    fread (cpu->code, sizeof (unsigned char), code_file_size, code_file);
    rewind (code_file);

    cpu->code_size = code_file_size - sizeof (code_info_t);
    cpu->code += sizeof (code_info_t);
}

int execute_code (cpu_t *cpu) {

    assert (cpu);

    if (verify_code_architecture (cpu->code - sizeof (code_info_t))) {

        return ERROR;
    }

    stack_t *stack_p = &(cpu->stack);
    addr_stack_t *addr_stack_p = &(cpu->addr_stack);
    for (cpu->ip = 0; cpu->ip < cpu->code_size; cpu->ip += sizeof (unsigned char)) {

        switch (cpu->code [cpu->ip] & ONLY_CMD_TYPE_MASK) {

            case HLT: {

                stack_dtor (stack_p);
                return SUCCESS;
            }

            case PUSH: {

                if (cpu->code [cpu->ip] & RAM_BIT_MASK) {

                    if (cpu->code [cpu->ip] & REG_BIT_MASK) {

                        if (cpu->code [cpu->ip] & IMM_BIT_MASK) {

                            stack_push (stack_p, cpu->ram [cpu->reg [*((unsigned char *) (cpu->code + cpu->ip + sizeof (unsigned char)))] + *((int *) (cpu->code + cpu->ip + 2 * sizeof (unsigned char)))]);
                            cpu->ip += sizeof (unsigned char) + sizeof (int);

                        } else {

                            stack_push (stack_p, cpu->ram [cpu->reg [*((unsigned char *) (cpu->code + cpu->ip + sizeof (unsigned char)))]]);
                            cpu->ip += sizeof (unsigned char);
                        }
                    } else {

                        stack_push (stack_p, cpu->ram [*((int *) (cpu->code + cpu->ip + sizeof (unsigned char)))]);
                        cpu->ip += sizeof (int);
                    }
                } else {

                    if (cpu->code [cpu->ip] & REG_BIT_MASK) {

                        if (cpu->code [cpu->ip] & IMM_BIT_MASK) {

                            stack_push (stack_p, cpu->reg [*((unsigned char *) (cpu->code + cpu->ip + sizeof (unsigned char)))] + *((int *) (cpu->code + cpu->ip + 2 * sizeof (unsigned char))));
                            cpu->ip += sizeof (unsigned char) + sizeof (int);

                        } else {

                            stack_push (stack_p, cpu->reg [*((unsigned char *) (cpu->code + cpu->ip + sizeof (unsigned char)))]);
                            cpu->ip += sizeof (unsigned char);
                        }
                    } else {

                        stack_push (stack_p, *((int *) (cpu->code + cpu->ip + sizeof (unsigned char))));
                        cpu->ip += sizeof (int);
                    }
                }
            
                break;
            }

            case POP: {

                if (cpu->code [cpu->ip] & RAM_BIT_MASK) {

                    if (cpu->code [cpu->ip] & REG_BIT_MASK) {

                        if (cpu->code [cpu->ip] & IMM_BIT_MASK) {

                            cpu->ram [cpu->reg [*((unsigned char *) (cpu->code + cpu->ip + sizeof (unsigned char)))] + *((int *) (cpu->code + cpu->ip + 2 * sizeof (unsigned char)))] = stack_pop (stack_p);
                            cpu->ip += sizeof (int) + sizeof (unsigned char);

                        } else {

                            cpu->ram [cpu->reg [*((unsigned char *) (cpu->code + cpu->ip + sizeof (unsigned char)))]] = stack_pop (stack_p);
                            cpu->ip += sizeof (unsigned char);
                        }
                    } else {

                        cpu->ram [*((int *) (cpu->code + cpu->ip + sizeof (unsigned char)))] = stack_pop (stack_p);
                        cpu->ip += sizeof (int);
                    }
                } else {
                    
                    if (cpu->code [cpu->ip] & REG_BIT_MASK) {

                        cpu->reg [*((unsigned char *) (cpu->code + cpu->ip + sizeof (unsigned char)))] = stack_pop (stack_p);
                        cpu->ip += sizeof (unsigned char);

                    } else {

                        stack_pop (stack_p);
                    }
                }

                break;
            }

            case ADD: {

                stack_push (stack_p, stack_pop (stack_p) + stack_pop (stack_p));
                break;
            }

            case SUB: {

                stack_push (stack_p, - stack_pop (stack_p) + stack_pop (stack_p));
                break;
            }

            case MUL: {

                stack_push (stack_p, stack_pop (stack_p) * stack_pop (stack_p));
                break;
            }

            case DIV: {

                int temp_val = stack_pop (stack_p);
                stack_push (stack_p, stack_pop (stack_p) / temp_val);
                break;
            }

            case IN: {

                int temp_val = 0;
                scanf ("%d", &temp_val);
                stack_push (stack_p, temp_val);
                break;
            }

            case OUT: {

                printf ("%d\n", stack_pop (stack_p));
                break;
            }

            case STVRF: {

                stack_verify (stack_p, *((int *) (cpu->code + cpu->ip + sizeof (unsigned char))));
                cpu->ip += sizeof (int);
                break;
            }

            case STDMP: {

                stack_dump (stack_p, *((int *) (cpu->code + cpu->ip + sizeof (unsigned char))));
                cpu->ip += sizeof (int);
                break;
            }

            case RGDMP: {

                reg_dump (cpu, *((int *) (cpu->code + cpu->ip + sizeof (unsigned char))));
                cpu->ip += sizeof (int);
                break;
            }

            case JMP: {

                cpu->ip = *((size_t *) (cpu->code + cpu->ip + sizeof (unsigned char))) - sizeof (unsigned char);
                break;
            }

            case JA: {

                int temp_val2 = stack_pop (stack_p), temp_val1 = stack_pop (stack_p);
                stack_push (stack_p, temp_val1);
                stack_push (stack_p, temp_val2);

                if (temp_val1 > temp_val2) {

                    cpu->ip = *((size_t *) (cpu->code + cpu->ip + sizeof (unsigned char))) - sizeof (unsigned char);
                }

                break;
            }

            case JAE: {

                int temp_val2 = stack_pop (stack_p), temp_val1 = stack_pop (stack_p);
                stack_push (stack_p, temp_val1);
                stack_push (stack_p, temp_val2);

                if (temp_val1 >= temp_val2) {

                    cpu->ip = *((size_t *) (cpu->code + cpu->ip + sizeof (unsigned char))) - sizeof (unsigned char);
                }

                break;
            }

            case JB: {

                int temp_val2 = stack_pop (stack_p), temp_val1 = stack_pop (stack_p);
                stack_push (stack_p, temp_val1);
                stack_push (stack_p, temp_val2);

                if (temp_val1 < temp_val2) {

                    cpu->ip = *((size_t *) (cpu->code + cpu->ip + sizeof (unsigned char))) - sizeof (unsigned char);
                }

                break;
            }

            case JBE: {

                int temp_val2 = stack_pop (stack_p), temp_val1 = stack_pop (stack_p);
                stack_push (stack_p, temp_val1);
                stack_push (stack_p, temp_val2);

                if (temp_val1 <= temp_val2) {

                    cpu->ip = *((size_t *) (cpu->code + cpu->ip + sizeof (unsigned char))) - sizeof (unsigned char);
                }

                break;
            }

            case JE: {

                int temp_val2 = stack_pop (stack_p), temp_val1 = stack_pop (stack_p);
                stack_push (stack_p, temp_val1);
                stack_push (stack_p, temp_val2);

                if (temp_val1 == temp_val2) {

                    cpu->ip = *((size_t *) (cpu->code + cpu->ip + sizeof (unsigned char))) - sizeof (unsigned char);
                }

                break;
            }

            case JNE: {

                int temp_val2 = stack_pop (stack_p), temp_val1 = stack_pop (stack_p);
                stack_push (stack_p, temp_val1);
                stack_push (stack_p, temp_val2);

                if (temp_val1 != temp_val2) {

                    cpu->ip = *((size_t *) (cpu->code + cpu->ip + sizeof (unsigned char))) - sizeof (unsigned char);
                }

                break;
            }

            case JF: {

                time_t tim = time (NULL);
                if (localtime (&tim)->tm_wday == FRIDAY) {

                    cpu->ip = *((size_t *) (cpu->code + cpu->ip + sizeof (unsigned char))) - sizeof (unsigned char);
                }

                break;
            }

            case CALL: {

                addr_stack_push (addr_stack_p, cpu->ip + sizeof (unsigned char));
                cpu->ip = *((size_t *) (cpu->code + cpu->ip + sizeof (unsigned char))) - sizeof (unsigned char);

                break;
            }

            case RET: {

                cpu->ip = addr_stack_pop (addr_stack_p) - sizeof (unsigned char);
                break;
            }

            case ASTDMP: {

                addr_stack_dump (addr_stack_p, *((int *) (cpu->code + cpu->ip + sizeof (unsigned char))));
                cpu->ip += sizeof (int);
                break;
            }

            case ASTVRF: {

                addr_stack_verify (addr_stack_p, *((int *) (cpu->code + cpu->ip + sizeof (unsigned char))));
                cpu->ip += sizeof (int);
                break;
            }

            default: {
                
                printf ("\nEXECUTION FAULT: WRONG COMMAND\n");
                stack_dtor (stack_p);
                return ERROR;
            }
        }
    }

    cpu->ip = 0;
    stack_dtor (stack_p);

    return SUCCESS;
}

static void reg_dump (cpu_t *cpu, int line) {

    FILE *log_file = fopen ("LOG.txt", "a");

    fprintf (log_file, "Register dump called by DUMP command (code line %d)\n\nRegisters address: [%p]\n\nValues stored:  ", line, &(cpu->reg));

    for (int i = 0; i < NUM_OF_REGS; i ++) {

        fprintf (log_file, "%cx: [%d]  ", 'a' + i, cpu->reg [i]);
    }

    fputs ("\n\n__________________________________\n\n\n", log_file);

    fclose (log_file);
}

static void clean_buffer_memory (unsigned char *code_buffer) {

    if (code_buffer) {

        free (code_buffer - sizeof (code_info_t));
    }
}
