#include "cpu.hpp"
#include "stack.hpp"

constexpr int NO_OFFSET = 0;

unsigned char *load_code (FILE *code_file, size_t *code_size);
void execute_code (unsigned char *code_buffer, size_t code_size);
void verify_launch_parameters (int argc);
void clean_memory (unsigned char *code_buffer);

static size_t get_in_file_size (FILE *file);
static void verify_code_architecture (unsigned char *code_buffer);

size_t get_file_size (FILE *file) {

    assert (file);

    fseek (file, NO_OFFSET, SEEK_END);
    size_t file_size = (size_t) ftell (file);

    rewind (file);
    return file_size;
}

void verify_launch_parameters (int argc) {

    if (argc != 2) {

        printf ("\nWrong input, please insert codefile name only\n");
        exit (EXIT_FAILURE);
    }
}

static void verify_code_architecture (unsigned char *code_buffer) {

    assert (code_buffer);

    if ((*((code_info_t *) code_buffer)).arch != ARCH_CISC) {

        printf ("\nEXECUTION FAULT: WRONG CODE ARCHITECTURE\n");
        exit (EXIT_FAILURE);
    }
}

unsigned char *load_code (FILE *code_file, size_t *code_size) {

    assert (code_file);
    assert (code_size);

    size_t code_file_size = get_file_size (code_file);

    unsigned char *code_buffer = (unsigned char *) calloc (code_file_size, sizeof (unsigned char));
    assert (code_buffer);
    fread (code_buffer, sizeof (unsigned char), code_file_size, code_file);
    rewind (code_file);

    verify_code_architecture (code_buffer);

    *code_size = code_file_size - sizeof (code_info_t);

    code_buffer += sizeof (code_info_t);
    return code_buffer;
}

void execute_code (unsigned char *code_buffer, size_t code_size) {

    assert (code_buffer);

    int reg [NUM_OF_REGS] = {};
    int ram [RAM_SIZE] = {};

    stack_t stack = {};
    stack_t *stack_p = &stack;
    stack_ctor (stack_p);

    for (size_t ip = 0; ip < code_size; ip += sizeof (unsigned char)) {

        switch (code_buffer [ip] & ONLY_CMD_TYPE_MASK) {

            case HLT: {

                stack_dtor (stack_p);
                return;
            }

            case PUSH: {

                if (code_buffer [ip] & RAM_BIT_MASK) {

                    if (code_buffer [ip] & REG_BIT_MASK) {

                        if (code_buffer [ip] & IMM_BIT_MASK) {

                            stack_push (stack_p, ram [reg [*((unsigned char *) (code_buffer + ip + sizeof (unsigned char)))] + *((int *) (code_buffer + ip + 2 * sizeof (unsigned char)))]);
                            ip += sizeof (unsigned char) + sizeof (int);

                        } else {

                            stack_push (stack_p, ram [(int *) reg [*((unsigned char *) (code_buffer + ip + sizeof (unsigned char)))]]);
                            ip += sizeof (unsigned char);
                        }
                    } else {

                        stack_push (stack_p, ram [(int *) *((int *) (code_buffer + ip + sizeof (unsigned char)))]);
                        ip += sizeof (int);
                    }
                } else {

                    if (code_buffer [ip] & REG_BIT_MASK) {

                        if (code_buffer [ip] & IMM_BIT_MASK) {

                            stack_push (stack_p, reg [*((unsigned char *) (code_buffer + ip + sizeof (unsigned char)))] + *((int *) (code_buffer + ip + 2 * sizeof (unsigned char))));
                            ip += sizeof (unsigned char) + sizeof (int);

                        } else {

                            stack_push (stack_p, reg [*((unsigned char *) (code_buffer + ip + sizeof (unsigned char)))]);
                            ip += sizeof (unsigned char);
                        }
                    } else {

                        stack_push (stack_p, *((int *) (code_buffer + ip + sizeof (unsigned char))));
                        ip += sizeof (int);
                    }
                }
            
                break;
            }

            case POP: {

                if (code_buffer [ip] & RAM_BIT_MASK) {

                    if (code_buffer [ip] & REG_BIT_MASK) {

                        if (code_buffer [ip] & IMM_BIT_MASK) {

                            ram [(int *) reg [*((unsigned char *) (code_buffer + ip + sizeof (unsigned char)))] + *((int *) (code_buffer + ip + 2 * sizeof (unsigned char)))] = stack_pop (stack_p);
                            ip += sizeof (int) + sizeof (unsigned char);

                        } else {

                            ram [(int *) reg [*((unsigned char *) (code_buffer + ip + sizeof (unsigned char)))]] = stack_pop (stack_p);
                            ip += sizeof (unsigned char);
                        }
                    } else {

                        ram [(int *) *((int *) (code_buffer + ip + sizeof (unsigned char)))] = stack_pop (stack_p);
                        ip += sizeof (int);
                    }
                } else {
                    
                    if (code_buffer [ip] & REG_BIT_MASK) {

                        reg [*((unsigned char *) (code_buffer + ip + sizeof (unsigned char)))] = stack_pop (stack_p);
                        ip += sizeof (unsigned char);

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

                int temp_val = stack_pop (stack_p);                        //  Деление нижнего элемента на верхний (аналогично вычитанию верхнего из нижнего)
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

                printf ("%d ", stack_pop (stack_p));
                break;
            }

            case STVRF: {

                stack_verify (stack_p, *((int *) (code_buffer + ip + 1)));
                ip += sizeof (int);
                break;
            }

            case STDMP: {

                stack_dump (stack_p, *((int *) (code_buffer + ip + 1)));
                ip += sizeof (int);
                break;
            }

            default: {
                printf ("\nEXECUTION FAULT: WRONG COMMAND\n");
                exit (EXIT_FAILURE);
            }
        }
    }

    stack_dtor (stack_p);
}

void clean_memory (unsigned char *code_buffer) {

    assert (code_buffer);

    free (code_buffer - sizeof (code_info_t));
}
