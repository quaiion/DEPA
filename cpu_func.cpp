#include "cpu.hpp"
#include "stack_func.cpp"

static size_t get_in_file_size (FILE *file);
static int verify_code_architecture (unsigned char *code_buffer);
static void clean_buffer_memory (unsigned char *code_buffer);

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

        if ((*((code_info_t *) code_buffer)).arch == 'NOPE') {

            printf ("\nDISASSEMBLING FAULT: CPU EMULATION IS DISTRUCTED\n");
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

                stack_verify (stack_p, *((int *) (cpu->code + cpu->ip + 1)));
                cpu->ip += sizeof (int);
                break;
            }

            case STDMP: {

                stack_dump (stack_p, *((int *) (cpu->code + cpu->ip + 1)));
                cpu->ip += sizeof (int);
                break;
            }

            default: {
                printf ("\nEXECUTION FAULT: WRONG COMMAND\n");
                return ERROR;
            }
        }
    }

    cpu->ip = 0;
    stack_dtor (stack_p);

    return SUCCESS;
}

void clean_buffer_memory (unsigned char *code_buffer) {

    if (code_buffer) {

        free (code_buffer - sizeof (code_info_t));
    }
}
