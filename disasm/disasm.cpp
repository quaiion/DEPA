#include <stdio.h>
#include "disasm.hpp"

int main (int argc, char *argv []) {

    verify_disasm_launch_parameters (argc);

    int no_err = SUCCESS;

    FILE *code_file = fopen (argv [1], "rb");
    size_t code_buffer_size = 0;
    unsigned char *code_buffer = store_code (code_file, &code_buffer_size);
    fclose (code_file);

    char *prog_buffer = (char *) malloc (DEFAULT_PROG_BUFFER_SIZE, sizeof (char));
    if (prog_buffer == NULL) {

        printf ("\nDISASSEMBLING FAULT: MEMORY ERROR\n");
        clean_disasm_memory (code_buffer, prog_buffer);
        return 0;
    }

    size_t disassembled_cmds_size = 0;
    no_err += disassemble_code (code_buffer, code_buffer_size, &prog_buffer, DEFAULT_PROG_BUFFER_SIZE, &disassembled_cmds_size);

    if (no_err == SUCCESS) {
        
        FILE *prog_file = open_prog_file (argc, argv);
        upload_prog (prog_buffer, prog_file, disassembled_cmds_size);
        fclose (prog_file);
        printf ("\nDISASSEMBLING FINISHED SUCCESSFULLY\n");
    }

    clean_disasm_memory (code_buffer, prog_buffer);
    return 0;
}
