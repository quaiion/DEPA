#include <stdio.h>
#include "disasm_func.cpp"

static void verify_launch_parameters (int argc);
static FILE *open_prog_file (int argc, char *argv []);

int main (int argc, char *argv []) {

    verify_launch_parameters (argc);

    int no_err = SUCCESS;

    FILE *code_file = fopen (argv [1], "rb");

    size_t code_buffer_size = 0;
    unsigned char *code_buffer = store_code (code_file, &code_buffer_size);
    fclose (code_file);

    size_t max_prog_size = estimate_prog_size (code_buffer, code_buffer_size);
    char *prog_buffer = (char *) calloc (max_prog_size, sizeof (char));
    if (prog_buffer == NULL) {

        printf ("\nDISASSEMBLING FAULT: MEMERY ERROR\n");
        exit (EXIT_FAILURE);
    }

    size_t disassembled_cmds_size = 0;
    no_err += disassemble_code (code_buffer, code_buffer_size, prog_buffer, &disassembled_cmds_size);

    if (no_err == SUCCESS) {
        
        FILE *prog_file = open_prog_file (argc, argv);
        upload_prog (prog_buffer, prog_file, disassembled_cmds_size);
        fclose (prog_file);

        printf ("\nDISASSEMBLING FINISHED SUCCESSFULLY\n");
    }

    clean_disasm_memory (code_buffer, prog_buffer);

    return 0;
}

static void verify_launch_parameters (int argc) {

    if (! (argc == 2 || argc == 3)) {

        printf ("\nWrong input, please insert codefile name and then (additionally) program file name only\n");
        exit (EXIT_FAILURE);
    }
}

static FILE *open_prog_file (int argc, char *argv []) {

    assert (argv);

    return (argc == 2) ? (fopen ("QO_prog_file.asm", "w")) : (fopen (argv [2], "w"));
}