#ifndef _EXPRESSION_CONVERT_H_
#define _EXPRESSION_CONVERT_H_

#include "core/func_convert.h"

void expr_tokenize(const char* std_code, struct token_stack* dst);
void expr_to_postfix(struct token_stack* infix, struct token_stack* postfix);
void expr_convert(struct token_stack* postfix, const struct variable_table* variable_table, const struct operation_table* operation_table, const struct function_table* function_table, char* return_type_dst, char* c_code);

#endif