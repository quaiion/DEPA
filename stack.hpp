#ifndef STACK_ACTIVE
#define STACK_ACTIVE

#include <stdio.h>
#include <malloc.h>
#include <limits.h>
#include <string.h>

struct stack_t {

    int *data;
    ssize_t capacity;
    ssize_t min_capacity /* >= 1 */;
    ssize_t size;
};

struct addr_stack_t {

    size_t *data;
    ssize_t capacity;
    ssize_t min_capacity /* >= 1 */;
    ssize_t size;
};

enum DUMP_CODE {NO_FLAWS =      0b00000000, NO_STK =    0b00000010, OVERFLOW =  0b00000100,                 \
                MIN_CAP_ERR =   0b01000000, MEM_ERR =   0b00010000, UNDERFLOW = 0b00000001,                 \
                SIZE_ERR =      0b00001000, CAP_ERR =   0b00100000};

constexpr unsigned char BROKEN_BYTE = 0xFF;
constexpr int OS_RESERVED_ADDRESS = 1;

/* main stack functions */
void stack_ctor (stack_t *stack, ssize_t min_capacity = 1);
void stack_dtor (stack_t *stack);

void stack_push (stack_t *stack, int value);
int stack_pop (stack_t *stack);

void stack_dump (stack_t *stack, int code_line);
void stack_verify (stack_t *stack, int code_line);

/* address stack functions */
void addr_stack_ctor (addr_stack_t *stack, ssize_t min_capacity = 1);
void addr_stack_dtor (addr_stack_t *stack);

void addr_stack_push (addr_stack_t *stack, int value);
size_t addr_stack_pop (addr_stack_t *stack);

void addr_stack_dump (addr_stack_t *stack, int code_line);
void addr_stack_verify (addr_stack_t *stack, int code_line);

#endif
