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
