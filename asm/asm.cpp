#include <stdio.h>
#include "asm.hpp"

int main (int argc, char *argv []) {

    if (verify_asm_launch_parameters (argc) ||
        verify_cmd_num () || verify_videomem ()) {

        return 0;
    }

    int no_err = SUCCESS;

    FILE *prog_file = fopen (argv [1], "r");
    if (prog_file == NULL) {

        printf ("\nNo .bin file in directory or no directory with such name, please try again\n");
        return 0;
    }
    FILE *code_file = open_code_file (argc, argv);
    if (code_file == NULL) {

        printf ("\nNo directory with such name, please try again\n");
    }

    char *cmd_buffer = store_cmds (prog_file);
    if (cmd_buffer == NULL) {

        return 0;
    }
    fclose (prog_file);

    unsigned long num_of_cmds = get_num_of_cmds (cmd_buffer);
    cmd_idx_t *cmd_index_tbl = index_cmds (cmd_buffer, num_of_cmds);
    if (cmd_index_tbl == NULL) {

        return 0;
    }

    size_t code_size = 0;
    mark_t *mark_tbl = NULL;
    unsigned num_of_marks = 0;
    no_err += preassemble_prog (cmd_index_tbl, num_of_cmds, &mark_tbl, &num_of_marks, &code_size);

    unsigned char *code_buffer = NULL;
    if (no_err == SUCCESS) {

        code_buffer = (unsigned char *) malloc (code_size * sizeof (unsigned char) + sizeof (code_info_t));
        if (code_buffer == NULL) {

            printf ("\nASSEMBLING FAULT: MEMORY ERROR\n");
            clean_asm_memory (cmd_index_tbl, cmd_buffer, code_buffer, mark_tbl);
            return 0;
        }

        set_code_info (&code_buffer);
        no_err += assemble_prog (cmd_index_tbl, num_of_cmds, code_buffer, mark_tbl, num_of_marks);
    }

    if (no_err == SUCCESS) {

        upload_code (code_buffer, code_file, code_size);
        fclose (code_file);

        printf ("\nASSEMBLING FINISHED SUCCESSFULLY\n");
    }

    clean_asm_memory (cmd_index_tbl, cmd_buffer, code_buffer, mark_tbl);
    return 0;
}
