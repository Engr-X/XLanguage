#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "core/type_convert.h"
#include "core/func_convert.h"
#include "lib/operation.h"
#include "lib/function.h"
#include "lib/utils.h"

#define __NATIVE_FUNCTION_COUNT 2

#define __ERROR_TYPE -1
#define __CAST_TYPE 0
#define __CONSTANT_TYPE 1
#define __VARIABLE_TYPE 2
#define __FUNCTION_TYPE 3

#define ARGS_SEPARATOR ", "
#define TYPES_SEPARATOR "_"

#define NATIVE_FUNCTION_PREFIX "__native_"
#define NATIVE_FUNCTION_ARGS_PREFIX "_args"

void token_type_to_string(int8_t raw_type, char* dst)
{
    switch (raw_type)
    {
        case TT_UNKNOWN: {sprintf(dst, "%s", "error for unknown"); break;}
        case TT_CONSTANT: {sprintf(dst, "%s", "constant"); break;}
        case TT_VARIABLE: {sprintf(dst, "%s", "variable"); break;}
        case TT_ARRAY: {sprintf(dst, "%s", "array"); break;}
        case TT_FUNCTION: {sprintf(dst, "%s", "function"); break;}
        case TT_OPERATION:{ sprintf(dst, "%s", "operation"); break;}
        case TT_DOT: {sprintf(dst, "%s", "dot"); break;}
        case TT_CAST: {sprintf(dst, "%s", "type_cast"); break;}
        default: {sprintf(dst, "%s", "error"); break;}
    }
}

void token_init(struct token_stack* stack)
{
    stack -> head = NULL;
    stack -> tail = NULL;
}

void token_add(struct token_stack* stack, char* token, uint8_t raw_type, uint8_t priority)
{
    struct token_node* node = (struct token_node*)(malloc(sizeof(struct token_node)));
    node -> token = utils_new_string(strlen(token) + 16);
    node -> type = NULL;
    node -> raw_type = raw_type;
    node -> priority = priority;
    node -> params_count = 0;
    strcpy(node -> token, token);

    node -> next = NULL;
    node -> prev = NULL;

    if (stack -> head == NULL)
    {
        stack -> head = node;
        stack -> tail = node;
    }
    else
    {
        node -> prev = stack -> tail;
        stack -> tail -> next = node;
        stack -> tail = node;
    }
}

void token_print(struct token_stack* stack)
{
    struct token_node* p = stack -> head;
    char* buffer = (char*)(malloc(sizeof(char) * 128));

    while (p != NULL)
    {
        token_type_to_string(p -> raw_type, buffer);
        printf("{token: %s}, {type: %s-%s}, {priority: %d}, {params: %d}\n",
                p -> token, buffer, p -> type == NULL ? "null" : p -> type, p -> priority, p -> params_count);
        p = p -> next;
    }

    free(buffer);
}

void token_free(struct token_node* node)
{
    if (node -> type != NULL)
        free(node -> type);

    free(node -> token);
}

void token_clear(struct token_stack* stack)
{
    struct token_node* p = stack -> head;

    while (p != NULL)
    {
        if (p -> type != NULL)
            free(p -> type);

        free(p -> token);
        struct token_node* temp = p;
        p = p -> next;
        free(temp);
    }
}

struct token_node* token_pop_front(struct token_stack* stack)
{
    if (stack == NULL || stack->head == NULL)
        return NULL;

    struct token_node* result = stack -> head;
    stack -> head = result -> next;

    if (stack -> head)
        stack -> head -> prev = NULL;
    else
        stack -> tail = NULL;

    result -> next = NULL;
    return result;
}

struct token_node* token_pop(struct token_stack* stack)
{
    struct token_node* result = stack -> tail;
    stack -> tail = result -> prev;

    if (stack -> tail)
        stack -> tail -> next = NULL;
    else
        stack -> head = NULL;

    result -> prev = NULL;

    return result;
}

static bool utils_is_number_digit(char c)
{
    return ('0' <= c && c <= '9') || c == '.';
}

static void analyze_plus_minus(const char* s, int8_t* dst)
{
    uint64_t len = strlen(s);
    memset(dst, 0, len);

    for (uint64_t i = 0; i < len; i++)
    {
        const char c = s[i];
        if (c != '+' && c != '-')
            continue;

        // ---------- detect ++ / -- ----------
        if ((c == '+' && s[i + 1] == '+') || (c == '-' && s[i + 1] == '-'))
        {
            // look back and forward
            int64_t j = (int64_t)i - 1;
            while (j >= 0 && (s[j] == ' ' || s[j] == '\t'))
                j--;

            uint64_t k = i + 2;
            while (k < len && (s[k] == ' ' || s[k] == '\t'))
                k++;

            bool is_prefix = false;

            // if before ++ is variable, number, or ] or )
            if (j >= 0 && (isalnum((unsigned char)s[j]) || s[j] == ']' || s[j] == ')'))
                is_prefix = false;
            // if before ++ is end/start of expr or operator
            else
                is_prefix = true;

            if (c == '+')
            {
                if (is_prefix)
                    dst[i] = dst[i + 1] = 4; // ++a
                else
                    dst[i] = dst[i + 1] = 3; // a++
            }
            else
            {
                if (is_prefix)
                    dst[i] = dst[i + 1] = -4; // --a
                else
                    dst[i] = dst[i + 1] = -3; // a--
            }

            i++; // skip one
            continue;
        }

        // ---------- detect single + / - ----------
        bool is_unary = false;

        int64_t j = (int64_t)i - 1;
        while (j >= 0 && (s[j] == ' ' || s[j] == '\t'))  // 只跳过空格和制表符
            j--;

        if (j < 0)
            is_unary = true;
        else
        {
            const char prev = s[j];
            // 判断是否在表达式结束之后
            bool is_expr_end = isalnum((unsigned char)prev) || prev == ']' || prev == ')';

            // 如果是后缀 ++ / -- 的结束位置 (dst=3/-3)，也视为表达式结束
            if (dst[j] == 3 || dst[j] == -3)
                is_expr_end = true;

            is_unary = !is_expr_end;  // 如果不是表达式结束，则为一元
        }

        dst[i] = (c == '+') ? (is_unary ? 1 : 2) : (is_unary ? -1 : -2);
    }
}

static void unwrap_cast(struct token_stack* dst)
{
    struct token_node* p = dst -> head;

    while (p != NULL)
    {
        if (p -> raw_type == TT_UNKNOWN)
        {
            if (p -> next != NULL)
            {
                const char next_first = p -> next -> token[0];

                if (next_first == '(')
                    p -> raw_type = TT_FUNCTION;
                else if (next_first == '[')
                    p -> raw_type = TT_ARRAY;
                else
                    p -> raw_type = TT_VARIABLE;
            }
            else
                p -> raw_type = TT_VARIABLE;
        }

        p = p -> next;
    }
}

static void recognize_type(struct token_stack* dst)
{
    struct token_node* p = dst -> head;

    while (p != NULL)
    {
        if (p -> raw_type == TT_UNKNOWN)
        {
            if (p -> next != NULL)
            {
                const char next_first = p -> next -> token[0];

                if (next_first == '(')
                    p -> raw_type = TT_FUNCTION;
                else if (next_first == '[')
                    p -> raw_type = TT_ARRAY;
                else
                    p -> raw_type = TT_VARIABLE;
            }
            else
                p -> raw_type = TT_VARIABLE;
        }

        p = p -> next;
    }
}

void func_tran_tokenize(const char* std_code, struct token_stack* dst)
{
    const char* p = std_code;
    char* buffer = utils_new_string(4096);
    int8_t* add_minus = (int8_t*)(malloc(strlen(std_code)));
    analyze_plus_minus(std_code, add_minus);

    while (*p != '\0')
    {
        const char c = *p;

        if (utils_is_space(c))
        {
            p++;
            continue;
        }

        switch (c)
        {
            case '\'':
            case '"':
            {
                const bool is_char = (c == '\'');
                const char* start = p;
                p++;

                while (*p != '\0')
                {
                    if (*p == '\\' && *(p + 1) != '\0')
                    {
                        p += 2;
                        continue;
                    }

                    if ((is_char && *p == '\'') || (!is_char && *p == '"'))
                        break;

                    p++;
                }

                utils_substring(start, 0, p - start + 1, buffer);
                token_add(dst, buffer, TT_CONSTANT, -1);
                break;
            }

            case '=':
            {
                if (*p + 1 != '\n')
                    printf_err("%serror occur: = occur in expression\n");

                p++;
                strcpy(buffer, "==");
                token_add(dst, buffer, TT_OPERATION, O_EQUAL);
                break;
            }

            case '!':
            {
                const bool not_equal_case = *(p + 1) == '='; // !=
                const bool xnor_case = *(p + 1) == '^';
                strcpy(buffer, not_equal_case ? "!=" : (xnor_case ? "!^" : "!"));
                token_add(dst, buffer, TT_OPERATION, xnor_case ? O_BITS_XNOR : O_BITS_REVERSE);

                if (not_equal_case)    
                    p++;

                break;
            }

            case '<':
            {
                const bool is_equal = *(p + 1) == '=';
                const bool is_shift = *(p + 1) == '<';

                strcpy(buffer, is_equal ? "<=" : (is_shift ? "<<" : "<"));
                token_add(dst, buffer, TT_OPERATION, is_equal ? O_LESS_EQUAL : (is_shift ? O_BITS_LEFT_SHIFT : O_LESS));

                if (is_equal || is_shift)
                    p++;

                break;
            }

            case '>':
            {
                const bool is_equal = *(p + 1) == '=';
                const bool is_shift = *(p + 1) == '>';

                strcpy(buffer, is_equal ? ">=" : (is_shift ? ">>" : ">"));
                token_add(dst, buffer, TT_OPERATION, is_equal ? O_GREATER_EQUAL : (is_shift ? O_BITS_RIGHT_SHIFT : O_GREATER));

                if (is_equal || is_shift)
                    p++;

                break;
            }

            case '*':
            {
                const bool is_power = *(p + 1) == '*';
                strcpy(buffer, is_power ? "**" : "*");
                token_add(dst, buffer, TT_OPERATION, is_power ? O_POW : O_MULTIPLY);   

                if (is_power)
                    p++;

                break;
            }

            case '+':
            {
                const uint64_t index = p - std_code;
                const int8_t result = add_minus[index];


                switch (result)
                {
                    case 1: // positive
                    case 2: // add
                    {
                        strcpy(buffer, "+");
                        token_add(dst, buffer, TT_OPERATION, result == 1 ? O_POSITIVE : O_ADD);   
                        break;
                    }
                    case 3: // self add
                    case 4: // add self
                    {
                        strcpy(buffer, "++");
                        token_add(dst, buffer, TT_OPERATION, result == 3 ? O_SELF_INCREASE : O_INCREASE_SELF);   
                        p++;
                        break;
                    }
                    default:
                    {
                        printf_err("%d error occur: unknown (analyze_plus_minus + break)\n", result);
                        break;
                    }
                }

                break;
            }

            case '-':
            {
                const uint64_t index = p - std_code;
                const int8_t result = add_minus[index];

                switch (result)
                {
                    case -1: // negative
                    case -2: // substract
                    {
                        strcpy(buffer, "-");
                        token_add(dst, buffer, TT_OPERATION, result == -1 ? O_NEGTIVE : O_SUBTRACT);   
                        break;
                    }
                    case -3: // self decrease
                    case -4: // self decrease self
                    {
                        strcpy(buffer, "--");
                        token_add(dst, buffer, TT_OPERATION, result == -3 ? O_SELF_DECREASE : O_DECREASE_SELF);   
                        p++;
                        break;
                    }
                    default: // 
                    {
                        printf_err("%d error occur: unknown (analyze_plus_minus - break)\n", result);
                        break;
                    }
                }
                
                break;
            }

            case '%':
            case '&':
            {
                const bool is_reverse = c == '!';
                const bool is_mod = c == '%';
                buffer[0] = c, buffer[1] = '\0';
                token_add(dst, buffer, TT_OPERATION, is_reverse ? O_BITS_REVERSE : (is_mod ? O_MOD : O_BITS_AND));
                break;
            }

            case '/':
            case '^':
            case '|':
            {
                const bool is_div = c == '/';
                const bool is_xor = c == '^';
                buffer[0] = c, buffer[1] = '\0';
                token_add(dst, buffer, TT_OPERATION, is_div ? O_DIVIDE : (is_xor ? O_BITS_XOR : O_BITS_OR));
                break;
            }

            case '(':
            case ')':
            case ',':
            case '[':
            case ']':
            {
                const bool is_dot = c == ',';
                const bool is_bracket = c == '(' || c == ')';
                buffer[0] = c, buffer[1] = '\0';
                token_add(dst, buffer, is_dot ? TT_DOT : TT_OPERATION, is_dot ? -1 : (is_bracket ? O_BRACKET : O_QUATATION));
                break;
            }

            default:
            {
               if (utils_is_number_digit(c))
                {
                    const char* start = p;
                    p++;
                    bool e = false;

                    while (*p != '\0')
                    {
                        const char c2 = *p;

                        if (utils_is_number_digit(c2) || c2 == '.')
                            p++;
                        else if (!e && (c2 == 'e' || c2 == 'E'))
                        {
                            e = true;
                            p++;

                            if (c2 == '+' || c2 == '-')
                                p++;
                        }
                        else if (c2 == 'f' || c2 == 'F')
                        {
                            p++;
                            break;
                        }
                        else if (c2 == 'l' || c2 == 'L')
                        {
                            p += (*(p + 1) == 'f' || *(p + 1) == 'F') ? 2 : 1;
                            break;
                        }
                        else
                            break;
                    }

                    utils_substring(start, 0, p - start, buffer);
                    token_add(dst, buffer, TT_CONSTANT, -1);
                    p--;
                    break;
                }
                
                if (utils_is_letter(c) || c == '_')
                {
                    const char* start = p;

                    while (*p != '\0' && (utils_is_letter(*p) || *p == '_' || utils_is_number_digit(*p) || *p == '.'))
                        p++;

                    utils_substring(start, 0, p - start, buffer);
                    const bool is_basic_type = type_is_basic_type(buffer);
                    const bool is_constant = (strcmp(buffer, "true") == 0) || (strcmp(buffer, "false") == 0);

                    token_add(dst, buffer, is_basic_type ? TT_CAST : (is_constant ? TT_CONSTANT : TT_UNKNOWN), is_basic_type ? O_CAST : -1);
                    p--;
                    
                    break;
                }
            }
        }

        p++;
    }

    recognize_type(dst);
    unwrap_cast(dst);

    free(buffer);
    free(add_minus);
}

static void pop_ops_until(struct token_stack* op, struct token_stack* postfix, char c, bool pop_bracket)
{
    bool is_strict = c == '(' || c == '[';
    bool is_bracket = c == '(';

    struct token_node* p = op -> tail;

    while (p != null)
    {
        const char c = *(p -> token);

        if (c == ')' || c == ']')
        {
            printf_err("Error: unbonded");
            return;
        }

        if (c == '(' || c == '[')
        {
            if (is_strict && is_bracket && c == '[')
                printf_err("Error: unmatch [");
            else if (is_strict && !is_bracket && c == '(')
                printf_err("Error: unmatch (");

            if (pop_bracket)
            {
                struct token_node* pop_one = token_pop(op);
                free(pop_one -> token);
                free(pop_one);
            }

            return;
        }

        struct token_node* pop_one = token_pop(op);
        token_add(postfix, pop_one -> token, pop_one -> raw_type, pop_one -> priority);
        postfix -> tail -> params_count = pop_one -> params_count;

        free(pop_one -> token);
        free(pop_one);

        p = op -> tail;
    }
}

void func_tran_to_postfix(struct token_stack* infix, struct token_stack* postfix)
{
    char* buffer = utils_new_string(256);
    struct token_stack* op = (struct token_stack*)(malloc(sizeof(struct token_stack)));
    struct token_node* temp = (struct token_node*)(malloc(sizeof(struct token_node)));
    token_init(op);

    if (infix -> head == NULL)
        return;

    struct token_node* p = infix -> head;

    while (p != NULL)
    {
        strcpy(buffer, p -> token);

        switch (p -> raw_type)
        {
            case TT_CONSTANT:
            case TT_VARIABLE:
            case TT_ARRAY:
            case TT_FUNCTION:
            {
                const bool is_number = (p -> raw_type == TT_CONSTANT || p -> raw_type == TT_VARIABLE);
                token_add(is_number ? postfix : op, p -> token, p -> raw_type, p -> priority);

                // detect functions args, func() has no, but func(2) has at leaset one
                if (p -> raw_type == TT_ARRAY || p -> raw_type == TT_FUNCTION)
                {
                    if (p -> next == NULL) // should be ( or [
                        printf_err("function's next param is null");
                    else if (p -> next -> next == NULL)
                        printf_err("function's( next param is null");
                    else
                    {
                        const char* token = p -> next -> next -> token;

                        if (strcmp(token, ")") != 0 && strcmp(token,  "]") != 0)
                            op -> tail -> params_count = 1;
                    }
                }
                
                // self add need push first and here is to push sign
                if (is_number && temp -> next == (void*)(1))
                {
                    token_add(postfix, temp -> token, temp -> raw_type, temp -> priority);
                    temp -> next = NULL; temp -> prev = NULL;
                }

                break;
            }
            
            // is operation
            case TT_CAST:
            case TT_OPERATION:
            {
                if (*buffer == '(' || *buffer == '[')
                    token_add(op, p -> token, p -> raw_type, p -> priority);
                else if (*buffer == ')' || *buffer == ']')
                {
                    //need pop （ and [ too!
                    pop_ops_until(op, postfix, *buffer, true);

                    // pop arr_name or function_name(f g)
                    if (op -> tail != NULL && (op -> tail -> raw_type == TT_ARRAY || op -> tail -> raw_type == TT_FUNCTION))
                    {
                        struct token_node* temp = token_pop(op);
                        token_add(postfix, temp -> token, temp -> raw_type, temp -> priority);
                        postfix -> tail -> params_count = temp -> params_count;
                        free(temp -> token);
                        free(temp);
                    }
                }
                else
                {
                    if (O_POSITIVE <= p -> raw_type && p -> raw_type <= O_SELF_DECREASE) // refer to positive negtive
                    {
                        if (temp != NULL)
                        {
                            printf_err("bug error for ++ --and + - ");
                            return;
                        }

                        memcpy(temp, p, sizeof(struct token_node));
                        temp -> next = (void*)(1); temp -> prev = (void*)(1);
                    }
                    else if (op -> head == NULL ||
                            *op -> tail -> token == '(' || *op -> tail -> token == '[' || *op -> tail -> token == '.' || 
                            p -> priority > op -> tail -> priority) // when op is empty or current priority greater than
                        token_add(op, p -> token, p -> raw_type, p -> priority);
                    else
                    {
                        while (op -> tail != NULL && p -> priority <= op -> tail -> priority)
                        {
                            if (*op -> tail -> token == '(' || *op -> tail -> token == '[')
                                break;

                            struct token_node* pop_one = token_pop(op);
                            token_add(postfix, pop_one -> token, pop_one -> raw_type, pop_one -> priority);
                            free(pop_one -> token);
                            free(pop_one);
                        }

                        token_add(op, p -> token, p -> raw_type, p -> priority);
                    }
                        
                    break;
                }

                break;
            }

            case TT_DOT:
            {
                //last must is ( or [
                struct token_node* tail = op -> tail;

                if (tail == NULL)
                {
                    printf_err("invalid dot position");
                    return;
                }

                if (tail -> prev == NULL)
                    break;

                //must is array or function_name
                struct token_node* tail_prev = tail -> prev; 

                if (tail_prev -> raw_type != TT_FUNCTION && tail_prev -> raw_type != TT_ARRAY)
                    pop_ops_until(op, postfix, -1, false);

                op -> tail -> prev -> params_count++;
                
                break;
            }
            default:
            {
                printf_err("error func_tran_to_postfix");
                break;
            }
        }

        //printf("add: %s\nop_stack:\n", p -> token);
        //token_print(op);
        //printf("\n%s", "output_stack:\n");
        //token_print(postfix);
        //puts("-----------");

        p = p -> next;
    }

    pop_ops_until(op, postfix, -1, false);

    token_clear(op);
    free(op);
    free(buffer);
    free(temp);
}

// warning direct prority
static void operation_set_params_acount(struct token_node* op)
{
    const char* token = op -> token;

    if (op -> raw_type == TT_OPERATION)
    {
        if (op -> priority == 10)
            op -> params_count = 1;
        else if (strcmp(token, "++") == 0 || strcmp(token, "--") == 0 || strcmp(token, "!") == 0)
            op -> params_count = 1;
        else
            op -> params_count = 2;
    }
}

static void classify_token(struct token_stack* postfix, const struct variable_table* variable_table)
{
    char* buffer = utils_new_string(16);
    struct token_node* p = postfix -> head;

    while (p != NULL)
    {
        switch (p -> raw_type)
        {
            case TT_UNKNOWN:
            {
                printf_err("cannot classify ");
                break;
            }

            case TT_CONSTANT:
            {
                p -> type = utils_new_string(16);
                variable_constant_type(p -> token, p -> type);
                break;
            }

            case TT_VARIABLE:
            {
                const bool contain_var = variable_contain(variable_table, p -> token);

                if (!contain_var)
                    printf_err("cannot find variable");
                else
                {
                    struct variable var;
                    variable_get(variable_table, p -> token, &var);
                    p -> type = utils_new_string(16);
                    strcpy(p -> type, var.type);
                }

                break;
            }

            case TT_OPERATION:
            {
                operation_set_params_acount(p);
                break;
            }

            default:
            {
                break;
            }
        }

        p = p -> next;
    }

    free(buffer);
}

void func_tran_convert(struct token_stack* postfix, const struct variable_table* variable_table, const struct operation_table* operation_table,
                        const struct function_table* function_table, char* return_type, char* c_code)
{
    char* buffer = utils_new_string(65535);
    struct token_node** arguments = (struct token_node**)(malloc(sizeof(struct token_node*) * 128));
    classify_token(postfix, variable_table);
    //token_print(postfix);

    struct operation op;
    struct token_stack* result_stack = (struct token_stack*)(malloc(sizeof(struct token_stack)));
    token_init(result_stack);

    while (postfix -> head != NULL)
    {
        memset(arguments, 0, sizeof(struct token_node*) * 128);

        struct token_node* p = token_pop_front(postfix);

        switch (p -> raw_type)
        {
            case TT_CONSTANT:
            case TT_VARIABLE:
            {
                token_add(result_stack, p -> token, p -> raw_type, p -> priority);
                result_stack -> tail -> type = utils_new_string(32);
                strcpy(result_stack -> tail -> type, p -> type);
                break;
            }

            case TT_CAST:
            {
                struct token_node* one = token_pop(result_stack);

                if (operation_contain(operation_table, NULL, p -> token, one -> type))
                {
                    operation_get(operation_table, NULL, p -> token, one -> type, &op);
                    operation_plugin(&op, NULL, one -> token, buffer);

                    token_add(result_stack, buffer, TT_CONSTANT, -1);
                    result_stack -> tail -> type = utils_new_string(32);
                    strcpy(result_stack -> tail -> type, op.return_type);
                }
                else
                    printf_err("cannot cast from: %s to: %s", one -> type, p -> token);

                break;
            }

            case TT_OPERATION:
            {
                if (p -> params_count == 2)
                {
                    struct token_node* second = token_pop(result_stack);
                    struct token_node* first = token_pop(result_stack);

                    if (operation_contain(operation_table, first -> type, p -> token, second -> type))
                    {
                        operation_get(operation_table, first -> type, p -> token, second -> type, &op);
                        operation_plugin(&op, first -> token, second -> token, buffer);

                        token_add(result_stack, buffer, TT_CONSTANT, -1);
                        result_stack -> tail -> type = utils_new_string(32);
                        strcpy(result_stack -> tail -> type, op.return_type);
                    }
                    else
                        printf_err("cannot find operation: %s %s %s", second -> type, p -> token, first -> type);

                    token_free(first); free(first);
                    token_free(second); free(second);
                }
                else if (p -> params_count == 1)
                {
                    struct token_node* one = token_pop(result_stack);
                    const bool is_prefix = (p -> priority == O_POSITIVE) || (p -> priority == O_INCREASE_SELF);

                    const char* first_type = is_prefix ? NULL : one -> type;
                    const char* second_type = is_prefix ? one -> type : NULL;
                    const char* first = is_prefix ? NULL : one -> token;
                    const char* second = is_prefix ? one -> token : NULL;
                    
                    if (operation_contain(operation_table, first_type, p -> token, second_type))
                    {
                        operation_get(operation_table, first_type, p -> token, second_type, &op);
                        operation_plugin(&op, first, second, buffer);

                        token_add(result_stack, buffer, TT_CONSTANT, -1);
                        result_stack -> tail -> type = utils_new_string(32);
                        strcpy(result_stack -> tail -> type, op.return_type);
                    }
                    else
                        printf_err("cannot find operation: %s %s %s", first_type == NULL ? "null" : first_type,
                                    p -> token, second_type == NULL ? "null" : second_type);
                    
                    token_free(one);
                    free(one);
                }

                break;
            }

            case TT_FUNCTION:
            {
                struct function func;
                uint16_t params_count = p -> params_count;

                for (uint16_t i = 0; i < params_count; i++)
                    arguments[i] = token_pop(result_stack);

                strcpy(buffer, "");
                for (int16_t i = params_count - 1; i >= 0; i--)
                {
                    strcat(buffer, "_");
                    strcat(buffer, arguments[i] -> type);
                }

                if (function_contain(function_table, p -> token, buffer))
                {
                    function_get(function_table, p -> token, buffer, &func);
                    strcpy(buffer, func.c_name);
                    strcat(buffer, "(");

                    for (int16_t i = params_count - 1; i >= 0; i--)
                    {
                        strcat(buffer, arguments[i] -> token);

                        if (i > 0)
                            strcat(buffer, ", ");

                        token_free(arguments[i]);
                        free(arguments[i]);
                    }

                    strcat(buffer, ")");

                    token_add(result_stack, buffer, TT_CONSTANT, -1);
                    result_stack -> tail -> type = utils_new_string(32);
                    strcpy(result_stack -> tail -> type, func.return_type);
                }
                else
                    printf_err("not function definition of %s, %s", p -> token, buffer);
            }
            
            default:
            {
                break;
            }
        }


        token_free(p);
        free(p);
    }
    
    strcpy(return_type, result_stack -> head -> type);
    strcpy(c_code, result_stack -> head -> token);
    token_clear(result_stack);
    free(result_stack);
    free(buffer);
    free(arguments);
}