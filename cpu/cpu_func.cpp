#include "cpu.hpp"

static int verify_in_num (float num);
static int verify_segmentation (int addr);
static int verify_stack_mem (stack_t *stack_p);
static int verify_addr_stack_mem (addr_stack_t *stack_p);

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

int verify_cpu_launch_parameters (int argc) {

    if (argc != 2) {

        printf ("\nWrong input, please insert codefile name only\n");
        return ERROR;
    }

    return SUCCESS;
}

int verify_cpu_code_signature (unsigned char *code_buffer) {

    assert (code_buffer);

    if ((*((code_info_t *) code_buffer)).sig != 'QO') {

        printf ("\nEXECUTION FAULT: INVALID FILE TYPE\n");
        return ERROR;
    }

    if ((*((code_info_t *) code_buffer)).arch != 'CISC') {

        if ((*((code_info_t *) code_buffer)).arch == 'NOPE') {

            printf ("\nEXECUTION FAULT: CPU EMULATION IS DISTRUCTED\n");
            return ERROR;
        }

        printf ("\nEXECUTION FAULT: INCONGRUENT CODE ARCHITECTURE\n");
        return ERROR;
    }

    return SUCCESS;
}

static int verify_in_num (float num) {

    if (((float) INT_MAX) / 1000 < num) {

        printf ("\nEXECUTION FAULT: MAX NUMBER ARGUMENT VALUE EXCEEDED\n");
        return ERROR;
    }

    return SUCCESS;
}

static int verify_segmentation (int addr) {

    if (addr < 0 || addr >= RAM_SIZE) {

        printf ("\nSEGMENTATION FAULT; EMULATION TERMINATED\n");
        return ERROR;
    }

    return SUCCESS;
}

static int verify_stack_mem (stack_t *stack_p) {

    if (stack_p->data == NULL) {

        printf ("\nNO MEM FOR STACK; EMULATION TERMINATED\n");
        return ERROR;
    }

    return SUCCESS;
}

static int verify_addr_stack_mem (addr_stack_t *addr_stack_p) {

    if (addr_stack_p->data == NULL) {

        printf ("\nNO MEM FOR STACK; EMULATION TERMINATED\n");
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

        printf ("\nLOADING FAULT: MEMORY ERROR\n");
        exit (EXIT_FAILURE);
    }

    fread (cpu->code, sizeof (unsigned char), code_file_size, code_file);
    rewind (code_file);

    cpu->code_size = code_file_size - sizeof (code_info_t);
    cpu->code += sizeof (code_info_t);
}

int execute_code (cpu_t *cpu) {

    assert (cpu);

    if (verify_cpu_code_signature (cpu->code - sizeof (code_info_t))) {

        return ERROR;
    }

    stack_t *stack_p = &(cpu->stack);
    addr_stack_t *addr_stack_p = &(cpu->addr_stack);
    for (cpu->ip = 0; cpu->ip < cpu->code_size; cpu->ip += sizeof (unsigned char)) {

        stop_signal_t signal = {};

        switch (cpu->code [cpu->ip] & ONLY_CMD_TYPE_MASK) {

#define CMD_PATTERN(name_cnst, token, arg_extraction_alg, arg_byte_size, execution_alg, preasm_format_alg, extern_arg, arg_assem_alg, arg_disas_print, max_disasm_arg_len) \
            case (name_cnst): { \
                \
                unsigned char *code_ptr = cpu->code + cpu->ip; \
                arg_extraction_alg \
                if (signal.stop) { \
                    \
                    break; \
                } \
                \
                size_t arg_size = arg_byte_size; \
                execution_alg \
                if (signal.stop) { \
                    \
                    break; \
                } \
                \
                cpu->ip += arg_byte_size; \
                break; \
            }

#include "../common/commands.hpp"
#undef CMD_PATTERN

            default: {
                
                printf ("\nEXECUTION FAULT: UNKNOWN COMMAND\n");
                return ERROR;
            }
        }

        if (signal.stop) {

            if (signal.err) {

                return ERROR;   // Состояние процессора не откатывается к исходному, чтобы можно было трейсить ошибки из мейна

            } else {

                break;
            }
        }
    }

    cpu->ip = 0;
    return SUCCESS;
}

void reg_dump (cpu_t *cpu, unsigned long line) {

    FILE *log_file = fopen ("cpu.log", "a");

    fprintf (log_file, "Register dump called by DUMP command (code line %lu)\n\nRegisters address: [%p]\n\nValues stored:  ", line, &(cpu->reg));

    for (int i = 0; i < NUM_OF_REGS; i ++) {

        fprintf (log_file, "%cx: [%d]  ", 'a' + i, cpu->reg [i]);
    }

    fputs ("\n\n__________________________________\n\n\n", log_file);

    fclose (log_file);
}

void clean_buffer_memory (unsigned char *code_buffer) {

    if (code_buffer) {

        free (code_buffer - sizeof (code_info_t));
    }
}
