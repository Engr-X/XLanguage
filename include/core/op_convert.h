#ifndef _OPERATION_CONVERT_H_
#define _OPERATION_CONVERT_H_

#include "lib/operation.h"

void op_convert(const struct operation_table* table, const char* left_type, const char* op, const char* right_type, char* buffer, char* dst);
#endif