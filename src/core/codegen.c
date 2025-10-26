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

static void codegen_iter(const struct statement_list* std_code, struct variable_table* variables,
                        struct operation_table* operations,
                        struct function_table* functions,
                        bool is_main,
                        char* varibale_buffer, char* expr_buffer, char* temp_buffer,
                        char* extern_dst, char* global_dst, char* init_dst, char* main_dst)
{
    struct statement_node* p = std_code -> head;

    struct token_stack token_stack;
    struct token_stack postfix;

    token_init(&token_stack);
    token_init(&postfix);

    char type_buffer[64]; *type_buffer = '\0';
    char name_buffer[64]; *name_buffer = '\0';

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
                {
                    break;
                }

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
                        expr_tokenize(code, &token_stack);
                        expr_to_postfix(&token_stack, &postfix);
                        expr_convert(&postfix, variables, operations, functions, type_buffer, expr_buffer);

                        strcat(main_dst, expr_buffer);
                        strcat(main_dst, ";\n");
                        break;
                    }
                    else // have assignment
                    {
                        utils_substring(code, 0, index, varibale_buffer); // left side
                        utils_substring(code, index + 1, strlen(code), expr_buffer); // right side

                        expr_tokenize(expr_buffer, &token_stack);
                        expr_to_postfix(&token_stack, &postfix);
                        expr_convert(&postfix, variables, operations, functions, type_buffer, expr_buffer);

                        struct string_list variable_def; stringlist_init(&variable_def);
                        utils_string_split(&variable_def, varibale_buffer, " \t\n\r\f\v");
                        bool is_const = false, is_extern = false;
                        struct string_node* p2 = variable_def.head;

                        while (p2 != NULL)
                        {
                            if (strcmp(p2 -> str, "const") == 0 || strcmp(p2 -> str, "final") == 0)
                                is_const = true;
                            else if (strcmp(p2 -> str, "extern") == 0 || strcmp(p2 -> str, "export") == 0)
                                is_extern = true;
                            else
                            {
                                strcpy(name_buffer, p2 -> str);
                                break;
                            }

                            p2 = p2 -> next;
                        }

                        stringlist_clear(&variable_def);
                        const bool is_defined = variable_contain(variables, name_buffer);

                        if (is_defined && (is_const || is_extern))
                        {
                            printf_err("variable %s has been defined\n", name_buffer);
                            token_clear(&postfix); token_clear(&token_stack);
                        }

                        if (is_defined)
                        {
                            struct variable* node = variable_get(variables, name_buffer);

                            if (node -> is_const)
                            {
                                printf_err("variable %s is const\n", name_buffer);
                                token_clear(&postfix); token_clear(&token_stack);
                            }
                        }

                        temp_buffer[0] = '\0';
                            
                        if (is_defined)
                        {
                            if (is_const)
                                printf_err("variable %s is const\n", name_buffer);

                            if (is_extern)
                                printf_err("variable %s is extern\n", name_buffer);
                            else
                            {
                                sprintf(temp_buffer, "%s = %s;\n", name_buffer, expr_buffer);
                                strcat(main_dst, temp_buffer);
                            }
                        }
                        else
                        {
                            if (is_extern)
                            {
                                if (is_const)
                                    printf_err("variable %s is cannot be const because it is extern\n", name_buffer);

                                variable_add(variables, name_buffer, type_buffer, false);
                                sprintf(temp_buffer, "extern %s %s;\n", type_buffer, name_buffer);
                                strcat(extern_dst, temp_buffer);

                                sprintf(temp_buffer, "%s %s;\n", type_buffer, name_buffer);
                                strcat(global_dst, temp_buffer);

                                sprintf(temp_buffer, "%s = %s;\n", name_buffer, expr_buffer);
                                strcat(init_dst, temp_buffer);
                            }
                            else
                            {
                                variable_add(variables, name_buffer, type_buffer, is_const);
                                sprintf(temp_buffer, "%s %s %s = %s;\n", is_const ? "const" : "", type_buffer, name_buffer, expr_buffer);
                                strcat(main_dst, temp_buffer);
                            }
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

            token_clear(&postfix); token_init(&postfix);
            token_clear(&token_stack); token_init(&token_stack);
        }   

        p = p -> next;
    }
}

static void codegen(const struct statement_list* c_code, char* head_code_dst, char* global_code_dst, char* init_dst, char* main_dst)
{
    struct variable_table* global_variables = (struct variable_table*)(malloc(sizeof(struct variable_table)));
    struct operation_table* gloablal_operations = (struct operation_table*)(malloc(sizeof(struct operation_table)));
    struct function_table* global_functions = (struct function_table*)(malloc(sizeof(struct function_table)));

    variable_init(global_variables);
    operation_init(gloablal_operations);
    function_init(global_functions);

    operation_add_default(gloablal_operations);
    function_add_default(global_functions);
    //function_print(global_functions);

    char* buffer1 = utils_new_string(4096);
    char* buffer2 = utils_new_string(4096);
    char* buffer3 = utils_new_string(4096);

    codegen_iter(c_code, global_variables, gloablal_operations, global_functions, true,
                 buffer1, buffer2, buffer3, head_code_dst, global_code_dst, init_dst, main_dst);

    variable_clear(global_variables); free(global_variables);
    operation_clear(gloablal_operations); free(gloablal_operations);
    function_clear(global_functions); free(global_functions);
    free(buffer1); free(buffer2); free(buffer3);
}

static void get_src(const char* file_name, const char* outer, const char* init, const char* main, char* dst)
{
    char format[1024] =
        "#include \"%s.h\"\n\n"

        "%s\n\n"

        "void %s_init(int argc, char const *argv[])\n"
        "{\n"
        "\t%s\n"
        "}\n\n"

        "int main(int argc, char const *argv[])\n"
        "{\n"
        "\t%s_init(argc, argv);\n"
        "\t%s\n"
        "\treturn 0;\n"
        "}";

    //printf("outer: %s\n", outer);
    //printf("init: %s\n", init);
    //printf("main: %s\n", main);

    sprintf(dst, format, file_name, outer, file_name, init, file_name, main);
}

static void get_header(const char* file_name, const char* head, char* dst)
{
    char upper_file_name[64];
    strcpy(upper_file_name, file_name);
    utils_to_upper(upper_file_name);

    char format[1024] =
        "#ifndef _X_%s_H_\n"
        "#define _X_%s_H_\n\n"

        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "#include <string.h>\n"
        "#include <math.h>\n"
        "#include <ctype.h>\n"
        "#include <stdbool.h>\n"
        "#include <time.h>\n"
        "#include <math.h>\n\n"

        "#include <native_io.h>\n"
        "#include <native_std.h>\n"
        "#include <native_constants.h>\n\n"

        "%s\n\n"

        "#endif";

    //printf("upper_file_name: %s\n", upper_file_name);
    //printf("head: %s\n", head);

    sprintf(dst, format, upper_file_name, upper_file_name, head);
}

void codegen_complete(const char* file_name, const char* x_code, char* head_dst, char* src_dst)
{
    struct statement_list statements;
    statement_init(&statements);
    codegen_separate(x_code, &statements, true, true);
    //statement_print(statements);

    char* head_code = utils_new_string(65535);
    char* global_code = utils_new_string(65536);
    char* init_code = utils_new_string(65536);
    char* main_code = utils_new_string(65536);

    codegen(&statements, head_code, global_code, init_code, main_code);

    get_header(file_name, head_code, head_dst);
    get_src(file_name, global_code, init_code, main_code, src_dst);

    statement_clear(&statements);

    free(head_code);
    free(global_code);
    free(init_code);
    free(main_code);
}