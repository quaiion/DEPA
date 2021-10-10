#include "processor_prototype.h"
#include <stdio.h>

int main (int argc, char** argv) {

    assert (argc > 1);

    stack_t oper_stk = {};
    stack_ctor (&oper_stk, sizeof (float));

    FILE* input_file = fopen (argv[1], "r");
    size_t flsize = get_file_size (input_file);

    char* bfr = NULL;
    int numofcmds = init_buffer (&bfr, input_file, flsize);

    char** indx_tbl = init_index_tbl (bfr, numofcmds);

    char* cmd = (char*) calloc (6, sizeof (char));
    float arg = 0;
    int obj_read = -1;
    float trashbin = 0;

    for (int i = 0; i < numofcmds; i++) {

        obj_read = sscanf (indx_tbl[i], "%s %f", cmd, &arg);

        if (strcmp (cmd, "hlt") == 0) {

            break;
        }

        if (strcmp (cmd, "push") == 0) {

            if (obj_read < 2) {

                continue;
            }

            stack_push (&oper_stk, &arg);

            continue;
        }

        if (strcmp (cmd, "pop") == 0) {

            stack_pop (&oper_stk, &trashbin);

            continue;
        }

        if (strcmp (cmd, "out") == 0) {             //  Печатаем с верха стека вниз

            printf ("\n");
            for (size_t i = 0; i < oper_stk.size; i++) {

                printf ("[%lld] %12.3f\n", i, *(((float*) oper_stk.data) + oper_stk.size - i - 1));
            }

            continue;
        }

        if (strcmp (cmd, "add") == 0) {

            if (oper_stk.size < 2) {

                continue;
            }

            float val1 = 0, val2 = 0, res = 0;

            stack_pop (&oper_stk, &val1);
            stack_pop (&oper_stk, &val2);

            res = val2 + val1;
            stack_push (&oper_stk, &res);

            continue;
        }

        if (strcmp (cmd, "sub") == 0) {

            if (oper_stk.size < 2) {

                continue;
            }

            float val1 = 0, val2 = 0, res = 0;

            stack_pop (&oper_stk, &val1);
            stack_pop (&oper_stk, &val2);

            res = val2 - val1;
            stack_push (&oper_stk, &res);

            continue;
        }

        if (strcmp (cmd, "mult") == 0) {

            if (oper_stk.size < 2) {

                continue;
            }

            float val1 = 0, val2 = 0, res = 0;

            stack_pop (&oper_stk, &val1);
            stack_pop (&oper_stk, &val2);

            res = val2 * val1;
            stack_push (&oper_stk, &res);

            continue;
        }

        if (strcmp (cmd, "div") == 0) {

            if (oper_stk.size < 2) {

                continue;
            }

            float val1 = 0, val2 = 0, res = 0;

            stack_pop (&oper_stk, &val1);
            stack_pop (&oper_stk, &val2);

            res = val2 / val1;
            stack_push (&oper_stk, &res);

            continue;
        }
    }

    stack_dtor (&oper_stk);
    fclose (input_file);
    clean_memory (indx_tbl, bfr, cmd);

    return 0;
}