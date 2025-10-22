#ifndef _FUNC_CONVERT_H_
#define _FUNC_CONVERT_H_

#include <stdbool.h>

#include "lib/variable.h"
#include "lib/operation.h"
#include "lib/function.h"

#define MAX_ARGC 32
#define MAX_ARGV_LENGTH 256

typedef enum toke_type
{
    TT_UNKNOWN = 0,
    TT_CONSTANT = 1,
    TT_VARIABLE = 2,
    TT_ARRAY = 3,
    TT_FUNCTION = 4,
    TT_OPERATION = 5,
    TT_DOT = 6,
    TT_CAST = 7
} TokenType;

typedef enum operation_priority
{
    O_ASSIGNMENT = 0,
    O_EQUAL = 1,                // ==   
    O_NOT_EQUAL = 1,            // !=   
    O_GREATER = 2,              // >    (微调：关系高于相等)
    O_LESS = 2,                 // <    
    O_GREATER_EQUAL = 2,        // >=   
    O_LESS_EQUAL = 2,           // <=   
    O_BITS_LEFT_SHIFT = 3,      // <<   (移位最低位组)
    O_BITS_RIGHT_SHIFT = 3,     // >>   
    O_BITS_OR = 4,              // |    (调整：| 最低)
    O_BITS_XOR = 5,             // ^    (中)
    O_BITS_XNOR = 5,            // !^
    O_BITS_AND = 6,             // &    (最高位运算)
    O_ADD = 7,                  // +
    O_SUBTRACT = 7,             // -
    O_MULTIPLY = 8,             // *    
    O_DIVIDE = 8,               // /    
    O_MOD = 8,                  // %    
    O_POW = 9,                  // **   
    O_POSITIVE = 10,             // <+>
    O_NEGTIVE = 10,              // <->
    O_BITS_REVERSE = 10,         // !   (调整到一元组，假设是~而非!)
    O_CAST = 10,                 // (type)  新增
    O_INCREASE_SELF = 11,       // ++c
    O_DECREASE_SELF = 11,       // --c
    O_SELF_INCREASE = 12,       // c++
    O_SELF_DECREASE = 12,       // c--
    O_BRACKET = 13,             // ()   
    O_QUATATION = 13            // []   
} OperationPriority;

typedef struct token_node
{
    char* token;
    char* type;

    uint8_t raw_type;
    uint8_t priority;
    uint16_t params_count;
    struct token_node* next;
    struct token_node* prev;
} TokenNode;

typedef struct token_stack
{
    struct token_node* head;
    struct token_node* tail;
} TokenStack;

void token_print(struct token_stack* stack);
void token_init(struct token_stack* stack);
void token_add(struct token_stack* stack, char* token, uint8_t raw_type, uint8_t priority);
void token_clear(struct token_stack* stack);
struct token_node* token_pop(struct token_stack* stack);

void func_tran_tokenize(const char* std_code, struct token_stack* dst);
void func_tran_to_postfix(struct token_stack* infix, struct token_stack* postfix);
void func_tran_translate(const char* std_code, struct function_table* func_table, struct variable_table* variable_table, char* dst);
bool is_native_function(const char* function_name, const char* signature);
void func_tran_convert(struct token_stack* postfix, const struct variable_table* variable_table, const struct operation_table* operation_table, const struct function_table* function_table, char* return_type_dst, char* c_code);

#endif