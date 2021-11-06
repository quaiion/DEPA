#include <stdio.h>
#include "disasm_func.cpp"

static void verify_launch_parameters (int argc);

int main (int argc, char *argv []) {

    verify_launch_parameters (argc);

    int no_err = 0;

    FILE *code_file = fopen (argv [1], "rb");

    size_t code_buffer_size = 0;
    unsigned char *code_buffer = store_code (code_file, &code_buffer_size);

    FILE *prog_file = open_prog_file (argc, argv);

    no_err += disassemble_code (code_buffer, code_buffer_size, prog_file);

    if (no_err == 0) {
        
        printf ("\nDISASSEMBLING FINISHED SUCCESSFULLY\n");
    }

    free (code_buffer - sizeof (code_info_t));
    fclose (code_file);
    fclose (prog_file);

    return 0;
}

void verify_launch_parameters (int argc) {

    if (! (argc == 2 || argc == 3)) {

        printf ("\nWrong input, please insert codefile name and then (additionally) program file name only\n");
        exit (EXIT_FAILURE);
    }
}