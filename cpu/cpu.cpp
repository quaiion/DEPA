#include <stdio.h>
#include "cpu.hpp"

int main (int argc, char *argv []) {

    if (verify_cpu_launch_parameters (argc) ||
        verify_cmd_num () || verify_videomem ()) {

        return 0;
    }

    cpu_t cpu = {};
    cpu_ctor (&cpu);

    FILE *code_file = fopen (argv [1], "rb");
    load_code (code_file, &cpu);
    fclose (code_file);

    execute_code (&cpu);

    cpu_dtor (&cpu);
    return 0;
}
