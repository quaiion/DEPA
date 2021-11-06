#include <stdio.h>
#include "cpu_func.cpp"

static void verify_launch_parameters (int argc);

int main (int argc, char *argv []) {

    verify_launch_parameters (argc);

    cpu_t cpu = {};
    cpu_ctor (&cpu);

    FILE *code_file = fopen (argv [1], "rb");

    load_code (code_file, &cpu);

    execute_code (&cpu);

    fclose (code_file);
    cpu_dtor (&cpu);
    
    return 0;
}

void verify_launch_parameters (int argc) {

    if (argc != 2) {

        printf ("\nWrong input, please insert codefile name only\n");
        exit (EXIT_FAILURE);
    }
}