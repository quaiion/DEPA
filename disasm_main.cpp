#include <stdio.h>
#include "disasm_func.cpp"

int main (int argc, char *argv []) {

    verify_launch_parameters (argc);

    FILE *code_file = fopen (argv [1], "r");

    size_t code_buffer_size = 0;
    unsigned char *code_buffer = store_code (code_file, &code_buffer_size);

    FILE *prog_file = open_prog_file (argc, argv);

    disassemble_code (code_buffer, code_buffer_size, prog_file);

    printf ("\nDISASSEMBLING FINISHED SUCCESSFULLY\n");

    free (code_buffer - sizeof (code_info_t));
    fclose (code_file);
    fclose (prog_file);

    return 0;
}

/*

В disassemble_code () ОЧЕНЬ нужна проверка на существование номера команды в массиве имен команд

*/