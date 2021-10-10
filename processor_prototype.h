#ifndef PROC_ACTIVE
#define PROC_ACTIVE

#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <limits.h>
#include <string.h>

enum CTOR_RES   {CTOR_SUCCESS = 0, CTOR_MEMERROR = -1, CTOR_RECNSTR = -2};
enum DTOR_RES   {DTOR_SUCCESS = 0, DTOR_MEMERROR = -1, DTOR_REDCNSTR = -2};
enum PUSH_RES   {PUSH_SUCCESS = 0, PUSH_MEMERROR = -1, PUSH_RESIZED = 1};
enum POP_RES    {POP_SUCCESS  = 0, POP_MEMERROR = -1, POP_RESIZED = 1, POP_STKEMPTY = 2};

constexpr int BROKEN_BYTE = 0xF000000;
constexpr int OS_RESERVED_ADDRESS = 1;
constexpr long NO_OFFSET = 0;

struct stack_t {

    char* data;
    unsigned int typesize;
    size_t capacity;
    size_t mincapacity;
    size_t size;
};

CTOR_RES stack_ctor (stack_t* stack, unsigned int typesize, size_t mincapacity = 0);
DTOR_RES stack_dtor (stack_t* stack);

PUSH_RES stack_push (stack_t* stack, void* value);
POP_RES stack_pop (stack_t* stack, void* value);

char* stack_resize_up (stack_t* stack);
char* stack_resize_down (stack_t* stack);

size_t get_file_size (FILE* const file);

int init_buffer (char** buffer, FILE* const file_in, const size_t filesize);
int num_of_cmds (const char* const buffer);
char** init_index_tbl (char* buffer, const int numofcmds);

void clean_memory (char** tbl, char* buffer, char* cmd);

#endif

/*

Расшарить %m, * и [] вместе с ним
Надо бы как-то запретить ползователю пользоваться стеком как структурой (или войдовать data, но тогда возникает вопрос, как видит войды calloc, memset, memcpy и free)

*/