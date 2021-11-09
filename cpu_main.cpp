#include <stdio.h>
#include "cpu_func.cpp"

static void verify_launch_parameters (int argc);

int main (int argc, char *argv []) {

    verify_launch_parameters (argc);

    cpu_t cpu = {};
    cpu_ctor (&cpu);

    FILE *code_file = fopen (argv [1], "rb");
    load_code (code_file, &cpu);
    fclose (code_file);

    execute_code (&cpu);

    cpu_dtor (&cpu);
    
    return 0;
}

static void verify_launch_parameters (int argc) {

    if (argc != 2) {

        printf ("\nWrong input, please insert codefile name only\n");
        exit (EXIT_FAILURE);
    }
}