#ifndef _LIBS_FUNCTION_H_
#define _LIBS_FUNCTION_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <uthash/uthash.h>

#include "lib/core_types.h"

#define MAX_X_FUNCTION_LENGTH 127
#define MAX_X_FUNCTION_ARGS_LENGTH 127
#define MAX_CLASS_LENGTH 31
#define MAX_C_FUNCTION_LENGTH 7

typedef struct function
{
    char* name;
    char* signature;
    char* return_type;
    
    char* hash_tag;

    char* c_name; // function name in C
    UT_hash_handle hh;
} Function;

typedef struct function_table 
{
    struct function* map;
} FunctionTable;

void function_init(struct function_table* table);
void function_add(struct function_table* table, const char* name, const char* signature, const char* return_type);
void function_add_native(struct function_table* table, const char* name, const char* c_name, const char* signature, const char* return_type);
void function_get(const struct function_table* table, const char* name, const char* signature, struct function* dst);
void function_remove(struct function_table* table, const char* name, const char* signature);
void function_print(const struct function_table* table);
void function_clear(struct function_table* table);
bool function_contain(const struct function_table* table, const char* name, const char* signature);
void function_add_default(struct function_table* dst_table);

#endif