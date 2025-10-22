#ifndef _LIBS_OPERATION_H_
#define _LIBS_OPERATION_H_

#include <stdint.h>

#include <uthash/uthash.h>

typedef enum position_type
{
    PREFIX,
    POSITFIX
} OperationType;

typedef struct operation
{
    uint8_t arity;
    uint8_t position;

    char* op;
    char* c_format;
    char* left_type;
    char* right_type;
    char* return_type;

    char* signature;

    UT_hash_handle hh;
} Operation;

typedef struct operation_table
{
    struct operation* map;
} OperationTable;

void operation_get_name(const char* left_type, const char* op, const char* right_type, char* dst);
void operation_plugin(const struct operation* node, const char* left, const char* right, char* dst);
void operation_print(const struct operation_table* table);

void operation_init(struct operation_table* table);
void operation_add(struct operation_table* table, const char* left_type,  const char* op, const char* right_type, const char* return_type);
void operation_add_native(struct operation_table* table, const char* left_type,  const char* op, const char* c_format, const char* right_type, const char* return_type);
void operation_clear(struct operation_table* table);
void operation_add_default(struct operation_table* dst_table);

void operation_get(const struct operation_table* table, const char* left, const char* op, const char* right, struct operation* dst);
bool operation_contain(const struct operation_table* table, const char* left, const char* op, const char* right);



#endif