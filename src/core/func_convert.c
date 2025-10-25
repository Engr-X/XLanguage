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
    *(node -> type) = '\0';
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
        printf("%p: {token: %s}, {type: %s-%s}, {priority: %d}, {params: %d}\n", p,
                p -> token, buffer, strlen(p -> type) == 0 ? "null" : p -> type, p -> priority, p -> params_count);
        p = p -> next;
    }

    free(buffer);
}

void token_clear(struct token_stack* stack)
{
    struct token_node* p = stack -> head;

    while (p != NULL)
    {
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