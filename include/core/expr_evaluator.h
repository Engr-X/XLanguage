#ifndef _EXPRESSION_EVALUATOR_H_
#define _EXPRESSION_EVALUATOR_H_

#include "core/expr_convert.h"

void expression_eval(struct token_stack postfix, char* result_dst);

#endif