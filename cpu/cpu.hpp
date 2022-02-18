#ifndef CPU_ACTIVE
#define CPU_ACTIVE

#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <malloc.h>
#include "cpu_stack.hpp"
#include "../common/common.hpp"

#define FRIDAY 5

struct cpu_t {

    unsigned char *code;
    int arch;
    int *ram;
    int reg [NUM_OF_REGS];
    stack_t stack;
    addr_stack_t addr_stack;
    size_t ip;
    size_t code_size;
};

void cpu_ctor (cpu_t *cpu);
void cpu_dtor (cpu_t *cpu);

void verify_cpu_launch_parameters (int argc);
int verify_cpu_code_signature (unsigned char *code_buffer);

void load_code (FILE *code_file, cpu_t *cpu);
int execute_code (cpu_t *cpu);
void clean_buffer_memory (unsigned char *code_buffer);
void reg_dump (cpu_t *cpu, unsigned long line);

#endif
