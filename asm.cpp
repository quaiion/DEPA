#include <stdio.h>
#include "asm.hpp"

int main (int argc, char *argv []) {

    verify_asm_launch_parameters (argc);

    int no_err = SUCCESS;

    FILE *prog_file = fopen (argv [1], "r");
    char *cmd_buffer = store_cmds (prog_file);
    fclose (prog_file);

    int num_of_cmds = get_num_of_cmds (cmd_buffer);
    char **cmd_index_tbl = index_cmds (cmd_buffer, num_of_cmds);

    mark_t *mark_tbl = NULL;
    int num_of_marks = 0;
    no_err += preassemble_prog (cmd_index_tbl, num_of_cmds, &mark_tbl, &num_of_marks);

    /*
    for (int i = 0; i < num_of_marks; i ++) {

        puts ((mark_tbl [i]).name);
        printf ("%lld\n\n", (mark_tbl [i]).idx);
    }
    */

    unsigned char *code_buffer = NULL;
    
    size_t assembled_code_size = 0;
    if (no_err == SUCCESS) {

        size_t max_code_size = estimate_code_size (cmd_index_tbl, num_of_cmds);
        code_buffer = (unsigned char *) calloc (max_code_size, sizeof (unsigned char));
        if (code_buffer == NULL) {

            printf ("\nASSEMBLING FAULT: MEMORY ERROR\n");
            exit (EXIT_FAILURE);
        }

        set_code_info (&code_buffer);

        no_err += assemble_prog (cmd_index_tbl, num_of_cmds, code_buffer, &assembled_code_size, mark_tbl, num_of_marks);
    }

    if (no_err == SUCCESS) {

        FILE *code_file = open_code_file (argc, argv);
        upload_code (code_buffer, code_file, assembled_code_size);
        fclose (code_file);

        printf ("\nASSEMBLING FINISHED SUCCESSFULLY\n");
    }

    clean_asm_memory (cmd_index_tbl, cmd_buffer, code_buffer, mark_tbl);
    
    return 0;
}
