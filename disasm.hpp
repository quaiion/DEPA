#ifndef DISASM_ACTIVE
#define DISASM_ACTIVE

#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "common.hpp"

FILE *open_prog_file (int argc, char *argv []);
unsigned char *store_code (FILE *code_file, size_t *code_buffer_size);
size_t estimate_prog_size (unsigned char *code_buffer, size_t code_buffer_size);
int disassemble_code (unsigned char* code_buffer, size_t code_buffer_size, char *prog_buffer, size_t *disassembled_cmds_size);
void upload_prog (char *prog_buffer, FILE *prog_file, size_t disassembled_cmds_size);
void clean_disasm_memory (unsigned char *code_buffer, char *prog_buffer);

void verify_disasm_launch_parameters (int argc);
int verify_disasm_code_signature (unsigned char *code_buffer);
int verify_cmd_code (size_t byte_number, unsigned char cmd_code);

#endif
