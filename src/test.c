#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "core/codegen.h"
#include "lib/function.h"
#include "lib/operation.h"
#include "lib/file_helper.h"
#include "lib/utils.h"
#include "core/expr_convert.h"

#include "math.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

void print_color()
{
    printf("\033[30mHello, 红色文字!\033[0m\n");
    printf("\033[32mHello, 绿色文字!\033[0m\n");
    printf("\033[33mHello, 黄色文字!\033[0m\n");
    printf("\033[34mHello, 蓝色文字!\033[0m\n");
}


void statment_test(const char* code)
{
    struct statement_list statement_list = {NULL, NULL};
    //codegen_preprocess(code, &statement_list, true, true);
    statement_print(&statement_list);
}

void token_test(const char* code)
{
    struct token_stack* token_stack = (struct token_stack*)(malloc(sizeof(struct token_stack)));
    struct token_stack* postfix = (struct token_stack*)(malloc(sizeof(struct token_stack)));
    token_init(token_stack);

    expr_tokenize(code, token_stack);
    expr_to_postfix(token_stack, postfix);
    token_clear(postfix);
    token_clear(token_stack);
    free(token_stack);
    free(postfix);
}

void function_test()
{
    struct function_table table;
    function_init(&table);
    function_add_default(&table);
    printf("%d", function_contain(&table, "min", "_int_int"));
    function_print(&table);
    function_clear(&table);
}

void postfix_test(const char* code)
{
    struct token_stack* token_stack = (struct token_stack*)(malloc(sizeof(struct token_stack)));
    struct token_stack* postfix = (struct token_stack*)(malloc(sizeof(struct token_stack)));

    token_init(token_stack);
    token_init(postfix);

    expr_tokenize(code, token_stack);
    expr_to_postfix(token_stack, postfix);

    token_print(postfix);

    token_clear(postfix);
    token_clear(token_stack);
    
    free(token_stack);
    free(postfix);
}

void translate_test(const char* code)
{
    char* return_type = utils_new_string(32);
    char* c_code = utils_new_string(65535);
    
    /*struct token_stack* token_stack = (struct token_stack*)(malloc(sizeof(struct token_stack)));
    struct token_stack* postfix = (struct token_stack*)(malloc(sizeof(struct token_stack)));
    struct operation_table* operation_table = (struct operation_table*)(malloc(sizeof(struct operation_table)));
    struct function_table* function_table = (struct function_table*)(malloc(sizeof(struct function_table)));

    token_init(token_stack);
    token_init(postfix);

    operation_init(operation_table);
    operation_add_default(operation_table);

    function_init(function_table);
    function_add_default(function_table);

    expr_tokenize(code, token_stack);
    expr_to_postfix(token_stack, postfix);
    expr_convert(postfix, NULL, operation_table, function_table, return_type, c_code);

    puts(c_code);

    token_clear(postfix);
    token_clear(token_stack);
    operation_clear(operation_table);
    function_clear(function_table);

    free(operation_table);
    free(function_table);

    free(token_stack);
    free(postfix);*/
    codegen_generate_c_code(code, c_code);

    free(c_code);
    free(return_type);
}

void operator_test()
{
    struct operation_table dst_table;
    operation_init(&dst_table);
    operation_add_default(&dst_table);

    struct operation* op;
    printf("conain: %d\n", operation_contain(&dst_table, "double", "^", "double"));

    operation_clear(&dst_table);
}

inline static void format_dual_arity1(const char* op, char* c_format)
{
    sprintf(c_format, "%%s %s %%s", op);
}

int main(int argc, char const *argv[])
{
    puts("start test");

    char* code = utils_new_string(2048);
    strcpy(code, "int 3\n");

    //codegen_separate(code, )

    //function_test();
    //operator_test();
    //function_test()
    //postfix_test(code);
    translate_test(code);
    //translate_test(code);
    free(code);
    return 0;
}