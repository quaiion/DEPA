#include <stdio.h>
#include "asm_func.cpp"

int main (int argc, char *argv []) {

    verify_launch_parameters (argc);

    FILE *prog_file = fopen (argv [1], "r");

    char *cmd_buffer = store_cmds (prog_file);
    int num_of_cmds = get_num_of_cmds (cmd_buffer);

    char **cmd_index_tbl = index_cmds (cmd_buffer, num_of_cmds);

    FILE *code_file = open_code_file (argc, argv);
    assemble_prog (cmd_index_tbl, num_of_cmds, code_file);

    printf ("\nASSEMBLING FINISHED SUCCESSFULLY\n");

    clean_memory (cmd_index_tbl, cmd_buffer);
    fclose (prog_file);
    fclose (code_file);
    
    return 0;
}