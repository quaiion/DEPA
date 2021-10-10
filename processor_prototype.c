#include "processor_prototype.h"

int format_buffer (char* buffer);

CTOR_RES stack_ctor (stack_t* stack, unsigned int typesize, size_t mincapacity) {

    assert (stack);

    CTOR_RES res_code = CTOR_SUCCESS;
    
    stack -> typesize = typesize;
    stack -> data = (char*) calloc (mincapacity, typesize);
    stack -> size = 0;
    stack -> mincapacity = mincapacity;
    
    if (stack -> data == NULL && mincapacity != 0) {

        stack -> capacity = 0;

        res_code = CTOR_MEMERROR;
        return res_code;
    }

    stack -> capacity = mincapacity;

    return res_code;
}

DTOR_RES stack_dtor (stack_t* stack) {

    assert (stack);

    DTOR_RES res_code = DTOR_SUCCESS;

    size_t stk_cap = stack -> capacity;

    stack -> size = SIZE_MAX;
    stack -> capacity = SIZE_MAX;
    stack -> mincapacity = SIZE_MAX;

    if (stack -> data == NULL && stk_cap != 0) {

        res_code = DTOR_MEMERROR;
        return res_code;
    }

    if (stack -> data) {

        memset (stack -> data, BROKEN_BYTE, stack -> typesize * stk_cap);
        free (stack -> data);
    }

    stack -> data = (char*) OS_RESERVED_ADDRESS;

    return res_code;
}

PUSH_RES stack_push (stack_t* stack, void* value) {

    assert (stack);
    assert (value);
    assert (stack -> size <= stack -> capacity);

    PUSH_RES res_code = PUSH_SUCCESS;

    if (stack -> data == NULL && stack -> capacity != 0) {

        res_code = PUSH_MEMERROR;
        return res_code;
    }

    if (stack -> size == stack -> capacity) {

        if (stack_resize_up (stack)) {

            res_code = PUSH_RESIZED;

        } else {

            res_code = PUSH_MEMERROR;
        }
    }

    memcpy (stack -> data + stack -> size ++ * stack -> typesize, (char*) value, stack -> typesize);

    return res_code;
}

POP_RES stack_pop (stack_t* stack, void* value) {

    assert (stack);
    assert (value);
    assert (stack -> size <= stack -> capacity);

    POP_RES res_code = POP_SUCCESS;

    if (stack -> data == NULL && stack -> capacity != 0) {

        res_code = POP_MEMERROR;
        return res_code;
    }

    if (stack -> size == 0) {

        res_code = POP_STKEMPTY;
        return res_code;
    }

    memcpy ((char*) value, stack -> data + -- stack -> size * stack -> typesize, stack -> typesize);

    if (stack -> size == (stack -> capacity - 3) / 4) {

        if (stack_resize_down (stack)) {

            res_code = POP_RESIZED;

        } else {

            res_code = POP_MEMERROR;
        }
    }

    return res_code;
}

char* stack_resize_up (stack_t* stack) {

    stack -> data = (char*) realloc (stack -> data, (stack -> capacity = stack -> capacity * 2 + 1) * stack -> typesize);

    return stack -> data;
}

char* stack_resize_down (stack_t* stack) {

    if (stack -> size >= stack -> mincapacity) {

        stack -> data = (char*) realloc (stack -> data, (stack -> capacity = (stack -> capacity - 1) / 2) * stack -> typesize);    
    }

    return stack -> data;
}

char** init_index_tbl (char* buffer, const int numofcmds) {

    assert (buffer);
    assert (numofcmds >= 0);

    char** indextbl = (char**) calloc (numofcmds, sizeof (char*));

    for (int i = 0; i < numofcmds; i++) {

        indextbl[i] = buffer;
        buffer = strchr (buffer, '\0') + 1;
    }

    return indextbl;
}

int init_buffer (char** buffer, FILE* const file_in, const size_t filesize) {
    
    assert (file_in);

    *buffer = (char*) calloc (filesize + 1, sizeof (char));

    int items_read = -1;
    items_read = fread (*buffer, sizeof (char), filesize, file_in);
    (*buffer)[items_read] = '\0';

    int num_of_cmds = -1;

    num_of_cmds = format_buffer (*buffer);

    rewind (file_in);
    return num_of_cmds;
}

size_t get_file_size (FILE* const file) {

    assert (file);

    fseek (file, NO_OFFSET, SEEK_END);

    size_t filesize = (size_t) ftell (file);

    rewind (file);
    return filesize;
}

int format_buffer (char* buffer) {

    int lines_done = -1;

    buffer = strchr (buffer, '\n');
    for (lines_done = 0; buffer; lines_done++) {

        *buffer = '\0';
        buffer++;
        buffer = strchr (buffer, '\n'); 
    }

    return lines_done;
}

void clean_memory (char** tbl, char* buffer, char* cmd) {

    assert (tbl);
    assert (buffer);
    assert (cmd);

    free (buffer);
    free (tbl);
    free (cmd);
}