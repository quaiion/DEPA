#include <stdio.h>
#include "asm_func.cpp"

static void verify_launch_parameters (int argc);

int main (int argc, char *argv []) {

    verify_launch_parameters (argc);

    int no_err = 0;

    FILE *prog_file = fopen (argv [1], "r");

    char *cmd_buffer = store_cmds (prog_file);
    int num_of_cmds = get_num_of_cmds (cmd_buffer);

    char **cmd_index_tbl = index_cmds (cmd_buffer, num_of_cmds);

    FILE *code_file = open_code_file (argc, argv);
    no_err += assemble_prog (cmd_index_tbl, num_of_cmds, code_file);

    if (no_err == 0) {

        printf ("\nASSEMBLING FINISHED SUCCESSFULLY\n");
    }

    clean_memory (cmd_index_tbl, cmd_buffer);
    fclose (prog_file);
    fclose (code_file);
    
    return 0;
}

void verify_launch_parameters (int argc) {

    if (! (argc == 2 || argc == 3)) {

        printf ("\nWrong input, please insert program file name and then (additionally) codefile name only\n");
        exit (EXIT_FAILURE);
    }
}