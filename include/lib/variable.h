#ifndef _LIBS_VARIABLE_H_
#define _LIBS_VARIABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <uthash/uthash.h>

#define MAX_X_VARIABLE_LENGTH 31
#define MAX_CLASS_LENGTH 31

typedef struct variable
{
    char name[64];
    char type[32];
    bool is_const;

    UT_hash_handle hh;
} Variable;

typedef struct variable_table 
{
    struct variable* map;
} VariableTable;

void variable_init(struct variable_table* table);
void variable_add(struct variable_table* table, const char* name, const char* type, bool is_const);
struct variable* variable_get(const struct variable_table* table, const char* name);
void variable_remove(struct variable_table* table, const char* name);
void variable_print(const struct variable_table* table);
void variable_clear(struct variable_table* table);
void variable_copy(const struct variable_table* table, struct variable_table* dst);
bool variable_contain(const struct variable_table* table, const char* name);



#endif