
#ifndef _TYPE_CONVERT_H_
#define _TYPE_CONVERT_H_

#include <stdbool.h>

extern const char* X_BASIC_TYPES[25];
extern const char* C_TYPES_NAMES[25];

void type_tran_print();
void type_tran_to_ctype(const char* xtype, char* dst);
bool type_is_basic_type(const char* xtype);
bool variable_is_constant(const char* x_code);
void variable_constant_tran(const char* x_code, char* dst);
void variable_constant_type(const char* x_code, char* dst);

#endif