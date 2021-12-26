#include "cpu_stack.hpp"
               
static void stack_resize_up (stack_t *stack);
static void stack_resize_down (stack_t *stack);
static void stack_verif_dump (stack_t *stack, int code_line_num, unsigned char dump_code);
static void addr_stack_resize_up (addr_stack_t *stack);
static void addr_stack_resize_down (addr_stack_t *stack);
static void addr_stack_verif_dump (addr_stack_t *stack, int code_line_num, unsigned char dump_code);

void stack_ctor (stack_t *stack, ssize_t min_capacity /* = 1 */) {

    stack->size = 0;
    stack->min_capacity = min_capacity;
    stack->capacity = min_capacity;
    stack->data = (int *) calloc (min_capacity, sizeof (int));
}

void stack_dtor (stack_t *stack) {

    memset (stack->data, BROKEN_BYTE, stack->capacity * sizeof (int));
    free (stack->data);

    stack->data = (int *) OS_RESERVED_ADDRESS;
    stack->size = SIZE_MAX;
    stack->capacity = SIZE_MAX;
    stack->min_capacity = SIZE_MAX;
}

void stack_push (stack_t *stack, int value) {

    if (stack->size == stack->capacity) stack_resize_up (stack);

    stack->data [stack->size ++] = value;
}

int stack_pop (stack_t *stack) {

    if (stack->size < 1) {          //  Единственная и простейшая проверка, которую делает сам стек - т.к. только такую ошибку (или юз фичи?) может сделать тот, кто пишет КОД, а не процессор

        return 0;
    }

    int value = stack->data [-- stack->size];

    if (stack->size == stack->capacity / 4 && stack->size >= stack->min_capacity) stack_resize_down (stack);

    return value;
}

static void stack_resize_up (stack_t *stack) {

    stack->data = (int *) realloc (stack->data, (stack->capacity = stack->capacity * 2) * sizeof (int));
}

static void stack_resize_down (stack_t *stack) {

    stack->data = (int *) realloc (stack->data, (stack->capacity = stack->capacity / 2) * sizeof (int));
}

void stack_dump (stack_t *stack, int code_line_num) {

    FILE *log_file = fopen ("LOG.txt", "a");

    fprintf (log_file, "Stack dump called by DUMP command (code line %d)", code_line_num);

    if (stack) {

        fprintf (log_file, "\n\nStack address: [%p]\n\nStack parameters:\nsize %lld\ncapacity %lld\nmin_capacity %lld\n\nData: [%p]\n",
                stack, stack -> size, stack -> capacity, stack -> min_capacity, stack -> data);

        ssize_t stack_cells_dumped = 0;
        for ( ; stack_cells_dumped < stack->size; stack_cells_dumped ++) {

            fprintf (log_file, "\n[%lld]   %10.d", stack_cells_dumped, stack->data [stack_cells_dumped]);
        }
        fputs (" ____ SIZE EDGE", log_file);

        for ( ; stack_cells_dumped < stack->capacity; stack_cells_dumped ++) {

            fprintf (log_file, "\n[%lld]   %10.d", stack_cells_dumped, stack->data [stack_cells_dumped]);
        }

    } else {

        fputs ("\n\n-----------------------------------\n\nWARNING: DUMP function found no stack address", log_file);
    }

    fputs ("\n\n__________________________________\n\n\n", log_file);

    fclose (log_file);
}

static void stack_verif_dump (stack_t *stack, int code_line_num, unsigned char dump_code) {

    FILE *log_file = fopen ("LOG.txt", "a");

    fprintf (log_file, "Stack dump automatically called by VERIFY command (code line %d)\n\nVerification codes returned:", code_line_num);
    
    if (dump_code == NO_FLAWS) {

        fputs ("\nCode 0: no flaws detected", log_file);
    }
        
    if ((dump_code & NO_STK) == NO_STK) {

        fputs ("\nCode -1: STACK pointer is null", log_file);
    }

    if ((dump_code & MEM_ERR) == MEM_ERR) {

        fputs("\nCode -2: actual stack's DATA pointer is null", log_file);
    }

    if ((dump_code & OVERFLOW) == OVERFLOW) {

        fputs ("\nCode -3: SIZE parameter of actual stack exceeds its CAPACITY parameter", log_file);
    }

    if ((dump_code & UNDERFLOW) == UNDERFLOW) {

        fputs ("\nCode -4: actual stack's CAPACITY value ran below min_capacity value", log_file);
    }

    if ((dump_code & CAP_ERR) == CAP_ERR) {

        fputs ("\nCode -5: actual stack's CAPACITY value ran below zero", log_file);
    }

    if ((dump_code & MIN_CAP_ERR) == MIN_CAP_ERR) {

        fputs ("\nCode -6: actual stack's MIN_CAPACITY value is set below zero", log_file);
    }

    if ((dump_code & SIZE_ERR) == SIZE_ERR) {

        fputs ("\nCode -7: actual stack's SIZE parameter ran below zero", log_file);
    }

    if (stack) {

        fprintf (log_file, "\n\nStack address: [%p]\n\nStack parameters:\nsize %lld\ncapacity %lld\nmin_capacity %lld\n\nData: [%p]\n",
                stack, stack -> size, stack -> capacity, stack -> min_capacity, stack -> data);

        ssize_t stack_cells_dumped = 0;
        for ( ; stack_cells_dumped < stack->size; stack_cells_dumped ++) {

            fprintf (log_file, "\n[%lld]   %10.d", stack_cells_dumped, stack->data [stack_cells_dumped]);
        }
        fputs (" ____ SIZE EDGE", log_file);

        for ( ; stack_cells_dumped < stack->capacity; stack_cells_dumped ++) {

            fprintf (log_file, "\n[%lld]   %10.d", stack_cells_dumped, stack->data [stack_cells_dumped]);
        }

    } else {

        fputs ("\n\n-----------------------------------\n\nWARNING: DUMP function found no stack address", log_file);
    }

    fputs ("\n\n__________________________________\n\n\n", log_file);

    fclose (log_file);
}

void stack_verify (stack_t *stack, int code_line_num) {

    unsigned char verif_code = NO_FLAWS;

    if (! stack) {
        
        verif_code |= NO_STK;
        stack_verif_dump (stack, code_line_num, verif_code);

        return;
    }

    if (stack->size < 0) {

        verif_code |= SIZE_ERR;
    }

    if (stack->size > stack->capacity) {

        verif_code |= OVERFLOW;
    }

    if (stack->min_capacity <= 0) {

        verif_code |= MIN_CAP_ERR;
    }

    if (stack->capacity <= 0) {

        verif_code |= CAP_ERR;
    }

    if (stack->capacity < stack->min_capacity) {

        verif_code |= UNDERFLOW;
    }

    if (stack->data == NULL) {

        verif_code |= MEM_ERR;
    }

    stack_verif_dump (stack, code_line_num, verif_code);
}

void addr_stack_ctor (addr_stack_t *stack, ssize_t min_capacity /* = 1 */) {

    stack->size = 0;
    stack->min_capacity = min_capacity;
    stack->capacity = min_capacity;
    stack->data = (size_t *) calloc (min_capacity, sizeof (size_t));
}

void addr_stack_dtor (addr_stack_t *stack) {

    memset (stack->data, BROKEN_BYTE, stack->capacity * sizeof (size_t));
    free (stack->data);

    stack->data = (size_t *) OS_RESERVED_ADDRESS;
    stack->size = SIZE_MAX;
    stack->capacity = SIZE_MAX;
    stack->min_capacity = SIZE_MAX;
}

void addr_stack_push (addr_stack_t *stack, size_t value) {

    if (stack->size == stack->capacity) addr_stack_resize_up (stack);

    stack->data [stack->size ++] = value;
}

size_t addr_stack_pop (addr_stack_t *stack) {

    if (stack->size < 1) {

        return 0;
    }

    size_t value = stack->data [-- stack->size];

    if (stack->size == stack->capacity / 4 && stack->size >= stack->min_capacity) addr_stack_resize_down (stack);

    return value;
}

static void addr_stack_resize_up (addr_stack_t *stack) {

    stack->data = (size_t *) realloc (stack->data, (stack->capacity = stack->capacity * 2) * sizeof (size_t));
}

static void addr_stack_resize_down (addr_stack_t *stack) {

    stack->data = (size_t *) realloc (stack->data, (stack->capacity = stack->capacity / 2) * sizeof (size_t));
}

void addr_stack_dump (addr_stack_t *stack, int code_line_num) {

    FILE *log_file = fopen ("LOG.txt", "a");

    fprintf (log_file, "ADDRESS Stack dump called by DUMP command (code line %d)", code_line_num);

    if (stack) {

        fprintf (log_file, "\n\nStack address: [%p]\n\nStack parameters:\nsize %lld\ncapacity %lld\nmin_capacity %lld\n\nData: [%p]\n",
                stack, stack -> size, stack -> capacity, stack -> min_capacity, stack -> data);

        ssize_t stack_cells_dumped = 0;
        for ( ; stack_cells_dumped < stack->size; stack_cells_dumped ++) {

            fprintf (log_file, "\n[%lld]   %10.lld", stack_cells_dumped, stack->data [stack_cells_dumped]);
        }
        fputs (" ____ SIZE EDGE", log_file);

        for ( ; stack_cells_dumped < stack->capacity; stack_cells_dumped ++) {

            fprintf (log_file, "\n[%lld]   %10.lld", stack_cells_dumped, stack->data [stack_cells_dumped]);
        }

    } else {

        fputs ("\n\n-----------------------------------\n\nWARNING: DUMP function found no stack address", log_file);
    }

    fputs ("\n\n__________________________________\n\n\n", log_file);

    fclose (log_file);
}

static void addr_stack_verif_dump (addr_stack_t *stack, int code_line_num, unsigned char dump_code) {

    FILE *log_file = fopen ("LOG.txt", "a");

    fprintf (log_file, "ADDRESS Stack dump automatically called by VERIFY command (code line %d)\n\nVerification codes returned:", code_line_num);
    
    if (dump_code == NO_FLAWS) {

        fputs ("\nCode 0: no flaws detected", log_file);
    }
        
    if ((dump_code & NO_STK) == NO_STK) {

        fputs ("\nCode -1: STACK pointer is null", log_file);
    }

    if ((dump_code & MEM_ERR) == MEM_ERR) {

        fputs("\nCode -2: actual stack's DATA pointer is null", log_file);
    }

    if ((dump_code & OVERFLOW) == OVERFLOW) {

        fputs ("\nCode -3: SIZE parameter of actual stack exceeds its CAPACITY parameter", log_file);
    }

    if ((dump_code & UNDERFLOW) == UNDERFLOW) {

        fputs ("\nCode -4: actual stack's CAPACITY value ran below min_capacity value", log_file);
    }

    if ((dump_code & CAP_ERR) == CAP_ERR) {

        fputs ("\nCode -5: actual stack's CAPACITY value ran below zero", log_file);
    }

    if ((dump_code & MIN_CAP_ERR) == MIN_CAP_ERR) {

        fputs ("\nCode -6: actual stack's MIN_CAPACITY value is set below zero", log_file);
    }

    if ((dump_code & SIZE_ERR) == SIZE_ERR) {

        fputs ("\nCode -7: actual stack's SIZE parameter ran below zero", log_file);
    }

    if (stack) {

        fprintf (log_file, "\n\nStack address: [%p]\n\nStack parameters:\nsize %lld\ncapacity %lld\nmin_capacity %lld\n\nData: [%p]\n",
                stack, stack -> size, stack -> capacity, stack -> min_capacity, stack -> data);

        ssize_t stack_cells_dumped = 0;
        for ( ; stack_cells_dumped < stack->size; stack_cells_dumped ++) {

            fprintf (log_file, "\n[%lld]   %10.lld", stack_cells_dumped, stack->data [stack_cells_dumped]);
        }
        fputs (" ____ SIZE EDGE", log_file);

        for ( ; stack_cells_dumped < stack->capacity; stack_cells_dumped ++) {

            fprintf (log_file, "\n[%lld]   %10.lld", stack_cells_dumped, stack->data [stack_cells_dumped]);
        }

    } else {

        fputs ("\n\n-----------------------------------\n\nWARNING: DUMP function found no stack address", log_file);
    }

    fputs ("\n\n__________________________________\n\n\n", log_file);

    fclose (log_file);
}

void addr_stack_verify (addr_stack_t *stack, int code_line_num) {

    unsigned char verif_code = NO_FLAWS;

    if (! stack) {
        
        verif_code |= NO_STK;
        addr_stack_verif_dump (stack, code_line_num, verif_code);

        return;
    }

    if (stack->size < 0) {

        verif_code |= SIZE_ERR;
    }

    if (stack->size > stack->capacity) {

        verif_code |= OVERFLOW;
    }

    if (stack->min_capacity <= 0) {

        verif_code |= MIN_CAP_ERR;
    }

    if (stack->capacity <= 0) {

        verif_code |= CAP_ERR;
    }

    if (stack->capacity < stack->min_capacity) {

        verif_code |= UNDERFLOW;
    }

    if (stack->data == NULL) {

        verif_code |= MEM_ERR;
    }

    addr_stack_verif_dump (stack, code_line_num, verif_code);
}