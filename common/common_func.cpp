#include "common.hpp"

size_t get_file_size (FILE *file) {

    assert (file);

    fseek (file, NO_OFFSET, SEEK_END);
    size_t file_size = (size_t) ftell (file);

    rewind (file);
    return file_size;
}

int get_num_of_digits_int (int number) {

    int num_of_digits = 0;

    if (number < 0) {

        num_of_digits = 1;
    }

    do {

        number /= 10;
        num_of_digits += 1;

    } while (number != 0);

    return num_of_digits;
}

int get_num_of_digits_sizet (size_t number) {

    int num_of_digits = 0;

    do {

        number /= 10;
        num_of_digits += 1;

    } while (number != 0);

    return num_of_digits;
}

int int_abs (int num) {

    return (num < 0) ? num * -1 : num;
}

int convert_float_to_int_1000 (float num_fl) {

    return (int) (num_fl * 1000);
}

float convert_int_1000_to_float (int num_int) {

    return (float) num_int / 1000;
}

int lin_search_str (const char *str, const char * const *arr, int nelem) {

    for (int i = 0; i < nelem; ++i) {

        if (strcmp (str, arr [i]) == STRINGS_EQUAL) {

            return i;
        }
    }

    return -1;
}

char *search_digit (char *str) {

    for (int i = 0; str [i] != '\0'; ++ i) {

        if (isdigit (str [i])) {

            return (str + i);
        }
    }

    return NULL;
}

int verify_cmd_num () {

    if (NUM_OF_CMD_TYPES > MAX_NUM_OF_CMD_TYPES) {

        printf ("\nMAX COMMANDS AMOUNT EXCEEDED; EMULATION TERMINATED\n");
        return ERROR;
    }

    return SUCCESS;
}

int verify_videomem () {

    if (VIDEOSEG_START + VIDEOSEG_SIZE > RAM_SIZE) {

        printf ("\nVIDEO SEGMENT DOESN NOT FIT IN THE MEM; EMULATION TERMINATED\n");
        return ERROR;
    }

    return SUCCESS;
}

int verify_reg_num (int reg) {

    if (reg < 0 || reg >= NUM_OF_REGS) {

        printf ("\nREGISTER FAULT; EMULATION TERMINATED\n");
        return ERROR;
    }

    return SUCCESS;
}
