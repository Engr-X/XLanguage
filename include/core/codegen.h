#ifndef _X_CORE_CODEGEN_H_
#define _X_CORE_CODEGEN_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct statement_node
{
    uint8_t type;
    char* code;
    struct statement_node* next;
} StatementNode;

typedef struct statement_list
{
    struct statement_node* head;
    struct statement_node* tail;
} StatementList;

void statement_print(struct statement_list* list);
void codegen_separate(const char* x_code, struct statement_list* dst, bool allow_class, bool allow_function);
void codegen_generate_c_code(const char* c_code, char* dst);

#endif