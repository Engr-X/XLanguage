#include "core/codegen.h"

#include "lib/utils.h"
#include "core/expr_convert.h"

#define CLASS_DEF_KEYWORD "class"
#define FUNCTION_DEF_KEYWORD "func"

enum statement_type
{
    CLASS_DEF = 0,
    FUNC_DEF = 1,
    IF = 2,
    IF_ELSE = 3,
    SWITCH_CASE = 4,
    LOOP = 5,
    ITERATION = 6,
    OTHER_OPERATION = 7
};

void statement_print(struct statement_list* list)
{
    struct statement_node* p = list -> head;

    char value[16] = "";

    while (p != NULL)
    {
    
        switch (p -> type)
        {
            case CLASS_DEF: {strcpy(value, "class"); break;}
            case FUNC_DEF: {strcpy(value, "function"); break;}
            case IF: {strcpy(value, "if"); break;}
            case IF_ELSE: {strcpy(value, "if-else"); break;}
            case SWITCH_CASE: {strcpy(value, "switch"); break;}
            case LOOP: {strcpy(value, "loop"); break;}
            case ITERATION: {strcpy(value, "iteration"); break;}
            default: {strcpy(value, "other"); break;}
        }
        
        printf("%s: [%s]\n", value, p -> code);
        p = p -> next;
    }
}

static void statement_init(struct statement_list* list)
{
    list -> head = NULL;
    list -> tail = NULL;
}

static void statement_add(struct statement_list* list, const char* code, uint8_t type)
{
    if (list -> head == NULL)
    {
        struct statement_node* node = (struct statement_node*)(malloc(sizeof(struct statement_node)));
        node -> code = utils_new_string(strlen(code) + 16);
        node -> type = type;

        strcpy(node -> code, code);
        node -> next = NULL;
        list -> head = node;
        list -> tail = node;
    }
    else
    {
        struct statement_node* node = (struct statement_node*)(malloc(sizeof(struct statement_node)));
        node -> code = utils_new_string(strlen(code) + 16);
        node -> type = type;
        
        strcpy(node -> code, code);
        node -> next = NULL;
        list -> tail -> next = node;
        list -> tail = node;
    }
}

static void statement_clear(struct statement_list* list)
{
    struct statement_node* p = list -> head;

    while (p != NULL)
    {
        free(p -> code);
        struct statement_node* temp = p;
        p = p -> next;
        free(temp);
    }
}

static char* next_block(const char* std_code_start)
{
    char* p = (char*)(std_code_start);
    bool first_bracket_find = false;
    int bracket_count = 0;

    while (*p != '\0' && (!first_bracket_find || bracket_count != 0))
    {
        if (*p == '"')
        {
            p++;

            while (*p && *p != '"')
                p++;

            if (*p)
                p++;

            continue;
        }

        if (*p == '\'')
        {
            p++;

            while (*p && *p != '\'')
                p++;

            if (*p)
                p++;

            continue;
        }

        if (*p == '{')
        {
            first_bracket_find = true;
            bracket_count++;
        }
        else if (*p == '}')
            bracket_count--;

        if (*p)
            p++;
    }

    return p;
}

void codegen_separate(const char* x_code, struct statement_list* dst, bool allow_class, bool allow_function)
{
    char* p = (char*)(x_code);
    char* prev_p = p;
    char* buffer = utils_new_string(65535);

    uint16_t bracket_count = 0;
    uint16_t square_count = 0;
    uint16_t brace_count = 0;

    while (*p != '\0')
    {
        while (*p != '\0' && utils_is_space(*p))
            p++;

        prev_p = p;
        bracket_count = 0;
        square_count = 0;
        brace_count = 0;

        // find first \n
        while (true)
        {
            if (*p == '\0' || *p == ';')
                break;

            if (*p == '\\')
            {
                p += 2;
                continue;
            }

            if (*p == '"')
            {
                p++;

                while (*p != '\0' && *p != '"')
                {
                    if (*p == '\\') {p += 2; continue;}
                    p++;
                }
            }

            if (*p == '\'')
            {
                p++;

                while (*p != '\0' && *p != '\'')
                {
                    if (*p == '\\') {p += 2; continue;}
                    p++;
                }
            }

            if (*p == '"' || *p == '\'') {p++;  continue;}
            if (brace_count == 0 && bracket_count == 0 && square_count == 0 && *p == '\n') break;

            switch (*p)
            {
                case '(': {bracket_count++; break;}
                case ')': {bracket_count--; break;}
                case '[': {square_count++; break;}
                case ']': {square_count--; break;}
                case '{': {brace_count++; break;}
                case '}': {brace_count--; break;}
            }

            p++;
        }

        if (*p == ';') p++;
        if (prev_p == p) continue;

        utils_substring(prev_p, 0, p - prev_p, buffer);
        //puts(buffer);

        const bool is_class_def = utils_code_contain(buffer, "class");
        const bool is_func_def = utils_code_contain(buffer, "func");

        if (is_class_def || is_func_def)
        {
            p = prev_p;
            p = next_block(p);
            utils_substring(prev_p, 0, p - prev_p, buffer);
            prev_p = p;
            statement_add(dst, buffer, is_class_def ? CLASS_DEF : FUNC_DEF);
            continue;
        }
        
        // if else statement
        const bool select = utils_code_contain(buffer, "?");
        const bool loop = utils_code_contain(buffer, "?->");
        const bool iteration = utils_code_contain(buffer, "->");

        if (select && !loop) // if statement
        {
            p = prev_p;
            p = next_block(p);
            char* temp = p;
            bool find_colon = false;

            while (true)
            {
                if (utils_is_space(*p))
                    p++;
                else
                {
                    find_colon = *p == ':';
                    break;
                }
            }

            if (find_colon)
            {
                p = next_block(p);
                utils_substring(prev_p, 0, p - prev_p, buffer);
                prev_p = p;
                statement_add(dst, buffer, IF_ELSE);
                printf("if-else: [%s]\n", buffer);
            }
            else
            {
                utils_substring(prev_p, 0, temp - prev_p, buffer);
                prev_p = p;
                bool is_switch = utils_code_contain(buffer, "default");
                statement_add(dst, buffer, is_switch ? SWITCH_CASE : IF);   
            }
        }
        else if (iteration || loop)
        {
            p = prev_p;
            p = next_block(p);
            utils_substring(prev_p, 0, p - prev_p, buffer);
            prev_p = p;
            statement_add(dst, buffer, loop ? LOOP : ITERATION);   
        } // for statement
        else // other
        {
            utils_substring(prev_p, 0, p - prev_p, buffer);
            prev_p = p;
            statement_add(dst, buffer, OTHER_OPERATION); 
        }
    }

    free(buffer);
}

static int get_assignment(const char* code)
{
    int16_t index = -1;
    
    // find first '=', but not "==". And it is not in "String" and 'char'
    for (char* iter = (char*)(code); *iter != '\0'; iter++)
    {
        if (*iter == '"')
        {
            iter++; // 跳过初始 "
            while (*iter != '\0')
            {
                if (*iter == '\\' && *(iter + 1) != '\0') iter++;
                else if (*iter == '"') break; // 找到真正的结束 "
                    iter++;
            }
        }
        else if (*iter == '\'')
        {
            iter++;

            while (*iter != '\0')
            {
                if (*iter == '\\' && *(iter+1) != '\0') iter++;
                else if (*iter == '\'') break;
                    iter++;
            }
        }
        else if (*iter == '=')
        {
            if (*(iter + 1) == '=')
            {
                iter++;
                continue;
            }
            else
            {
                index = iter - code;
                break;
            }
        }
    }

    return index;
}

static void get_left_right_var(const char* line, char* left_buffer, char* right_buffer)
{
    char* p = (char*)(line);

    while (*p != '\0' && utils_is_space(*p))
        p++;

    const char* start = p;

    while (*p != '\0' && (utils_is_letter(*p) || utils_is_digit(*p) || *p == '_' || *p == '.'))
        p++;
    
    utils_substring(start, 0, p - start, left_buffer);
    utils_substring(p, 0, strlen(p), right_buffer);
}

static void codegen_convert(struct statement_list* c_code, char* other_dst, char* main_dst)
{
    struct variable_table* global_variables = (struct variable_table*)(malloc(sizeof(struct variable_table)));
    struct operation_table* gloablal_operations = (struct operation_table*)(malloc(sizeof(struct operation_table)));
    struct function_table* global_functions = (struct function_table*)(malloc(sizeof(struct function_table)));

    struct token_stack* token_stack = (struct token_stack*)(malloc(sizeof(struct token_stack)));
    struct token_stack* postfix = (struct token_stack*)(malloc(sizeof(struct token_stack)));
    token_init(token_stack);
    token_init(postfix);
    
    variable_init(global_variables);
    operation_init(gloablal_operations);
    function_init(global_functions);

    operation_add_default(gloablal_operations);
    function_add_default(global_functions);
    //function_print(global_functions);

    struct statement_node* p = c_code -> head;

    char* buffer1 = utils_new_string(2048);
    char* buffer2 = utils_new_string(1024);
    char* buffer3 = utils_new_string(1024);
    char type_buffer[64];
    char variable_buffer1[64];
    char variable_buffer2[64];
    char* std_code = utils_new_string(8192);

    while (p != NULL)
    {
        const char* code = p -> code;

        if (utils_string_contain(code, CLASS_DEF_KEYWORD))
        {
            //puts("define class");
        }
        else if (utils_string_contain(code, FUNCTION_DEF_KEYWORD))
        {
            //puts("define function");
        }
        else // other
        {
            switch (p -> type)
            {
                case IF:
                case IF_ELSE:
                case SWITCH_CASE:
                case LOOP:
                case ITERATION:
                {
                    puts("not implemented yet");
                    break;
                }

                case OTHER_OPERATION:
                {
                    const int16_t index = get_assignment(code);

                    if (index == -1) // function call or operation without assignment
                    {
                        expr_tokenize(code, token_stack);
                        expr_to_postfix(token_stack, postfix);
                        expr_convert(postfix, global_variables, gloablal_operations, global_functions, type_buffer, buffer1);
                        strcat(main_dst, buffer1);
                        strcat(main_dst, ";\n");
                        break;
                    }
                    else // have assignment
                    {
                        utils_substring(code, 0, index, buffer3); // left side
                        utils_substring(code, index + 1, strlen(code), buffer2); // right side

                        expr_tokenize(buffer2, token_stack);
                        expr_to_postfix(token_stack, postfix);
                        expr_convert(postfix, global_variables, gloablal_operations, global_functions, type_buffer, buffer1);

                        get_left_right_var(buffer3, variable_buffer1, variable_buffer2);

                        if (variable_contain(global_variables, variable_buffer1))
                        {
                            puts("change assignment");
                        }
                        else
                        {
                            puts("new assignment");
                            variable_add(global_variables, variable_buffer1, type_buffer);
                            strcat(main_dst, type_buffer);
                            strcat(main_dst, " ");
                            strcat(main_dst, variable_buffer1);
                            strcat(main_dst, " = ");
                            strcat(main_dst, buffer1);
                            strcat(main_dst, ";\n");
                        }

                        break;   
                    }
                }

                default:
                {
                    printf_err("unknown statement type %d\n", p -> type);
                    break;
                }
            }

            token_clear(postfix);
            token_clear(token_stack);
            token_init(token_stack);
            token_init(postfix);
        }

        p = p -> next;
    }

    variable_clear(global_variables); operation_clear(gloablal_operations); function_clear(global_functions);
    free(global_variables); free(gloablal_operations); free(global_functions);
    free(token_stack); free(postfix);

    free(std_code);
    free(buffer1);
    free(buffer2);
    free(buffer3);
}

void codegen_generate_c_code(const char* x_code, char* dst)
{
    strcpy(dst, "#include <stdio.h>\n");
    strcat(dst, "#include <stdlib.h>\n");
    strcat(dst, "#include <string.h>\n");
    strcat(dst, "#include <math.h>\n");
    strcat(dst, "#include <ctype.h>\n");
    strcat(dst, "#include <stdbool.h>\n");
    strcat(dst, "#include <time.h>\n");
    strcat(dst, "#include <math.h>\n\n");

    strcat(dst, "#include <native_io.h>\n");
    strcat(dst, "#include <native_std.h>\n");
    strcat(dst, "#include <native_constants.h>\n\n");

    struct statement_list* statements = (struct statement_list*)(malloc(sizeof(struct statement_list)));
    statement_init(statements);
    codegen_separate(x_code, statements, true, true);
    statement_print(statements);

    char* other_dst = (char*)(malloc(65536 * sizeof(char)));
    char* main_dst = (char*)(malloc(65536 * sizeof(char)));
    *other_dst = '\0';
    *main_dst = '\0';

    codegen_convert(statements, other_dst, main_dst);

    strcat(dst, other_dst);
    strcat(dst, "int main(int argc, char const *argv[])\n{\n");
    strcat(dst, "srand(time(NULL));\n\n");

    strcat(dst, main_dst);
    strcat(dst, "\n");

    strcat(dst, "return 0;\n}");

    statement_clear(statements);
    
    free(statements);
    free(other_dst);
    free(main_dst);
}