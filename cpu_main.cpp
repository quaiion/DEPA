#include <stdio.h>
#include "cpu_func.cpp"

int main (int argc, char *argv []) {

    verify_launch_parameters (argc);

    FILE *code_file = fopen (argv [1], "r");

    size_t code_size = 0;
    unsigned char *code_buffer = load_code (code_file, &code_size);

    execute_code (code_buffer, code_size);

    clean_memory (code_buffer);
    fclose (code_file);
    
    return 0;
}