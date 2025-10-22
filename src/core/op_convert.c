#include <stdio.h>

#include "core/op_convert.h"
#include "lib/utils.h"



void op_convert(const struct operation_table* table, const char* left_type, const char* op, const char* right_type, char* return_type, char* dst)
{
    char* buffer = utils_new_string(128);
    struct operation op;
    operation_get(table, left_type, op, right_type, &op);
    free(buffer);
}