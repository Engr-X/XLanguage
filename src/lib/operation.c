#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "core/type_convert.h"
#include "lib/operation.h"
#include "lib/utils.h"

#define OP_SIZE 29

static const char* OP_SYMBOLS[OP_SIZE] =
{
    "==", "!=", ">", "<", ">=", "<=", "<<", ">>", "|", "^",
    "!^", "&", "+", "-", "*", "/", "%%", "%", "**", "<+>", "<->",
    "~", "cast", "++", "--","++", "--", "()", "[]"
};

static const char* OP_NAME[OP_SIZE] =
{
    "equal", "not_equal", "greater", "less", "greater_equal", "less_equal", "bits_left_shift", "bits_right_shift", "bits_or", "bits_xor",
    "bits_xnor", "bits_and", "add", "subtract", "multiply", "divide", "mod", "mod", "pow", "positive", "negtive",
    "bits_reverse", "cast", "increase_self", "decrease_self", "self_increase", "self_decrease", "bracket", "quatation"
};

static void try_free(void* memory)
{
    if (memory != NULL)
        free(memory);
}
static void change_name(const char* op, char* dst)
{
    for (uint8_t i = 0; i < OP_SIZE; i++)
    {
        if (strcmp(op, OP_SYMBOLS[i]) == 0)
        {
            strcpy(dst, OP_NAME[i]);
            return;
        }
    }
    
    strcpy(dst, op);
    return;
}

void operation_get_name(const char* left_type, const char* op, const char* right_type, char* dst)
{
    char* buffer = utils_new_string(256);
    change_name(op, buffer);

    strcpy(dst, "");

    strcat(dst, left_type == NULL ? "null" : left_type);
    strcat(dst, "_");
    strcat(dst, buffer);
    strcat(dst, "_");
    strcat(dst, right_type == NULL ? "null" : right_type);
    free(buffer);
}

void operation_plugin(const struct operation* node, const char* left, const char* right, char* dst)
{
    if (node -> arity == 2)
        sprintf(dst, node -> c_format, left, right);
    else if (node -> arity == 1 && node -> position == PREFIX)
        sprintf(dst, node -> c_format, right);
    else if (node -> arity == 1 && node -> position == POSITFIX)
        sprintf(dst, node -> c_format, left);
    else
        printf_err("err");
}

void operation_print(const struct operation_table* table)
{
    int i = 0;
    char* buffer = utils_new_string(128);
    struct operation* v;

    for (v = table -> map; v != NULL; v = (struct operation*)(v -> hh.next))
    {
        const bool left_bool = v -> left_type == NULL;
        const bool right_bool = v -> right_type == NULL;
        operation_plugin(v, "value1", "value2", buffer);
        printf("%d: %s %s %s = %s %s\n", i, left_bool ? "null" : v -> left_type, v -> op, right_bool ? "null" : v -> right_type, v -> return_type, buffer);
        i++;
    }

    free(buffer);
}

void operation_init(struct operation_table* table)
{
    table -> map = NULL;
}

void operation_add(struct operation_table* table, const char* left_type,  const char* op, const char* right_type, const char* return_type)
{
    char* signature = utils_new_string(256);
    operation_get_name(left_type, op, right_type, signature);

    struct operation* v = NULL;

    HASH_FIND_STR(table -> map, signature, v);

    if (v == NULL)
    {
        v = (struct operation*)(malloc(sizeof(struct operation)));
        const bool left_null = left_type == NULL;
        const bool right_null = right_type == NULL;

        v -> op = utils_new_string(64);
        v -> left_type = left_null ? NULL : utils_new_string(32);
        v -> right_type = left_null ? NULL :  utils_new_string(32);
        v -> return_type = utils_new_string(32);
        v -> signature = utils_new_string(256);
        v -> c_format = NULL;

        if ((!left_null && right_null) || (left_null && !right_null))
        {
            v -> arity = 1;
            v -> position = left_null ? PREFIX : POSITFIX;
        }
        else if (!left_null && !right_null)
            v -> arity = 2;

        strcpy(v -> op, op);
        if (!left_null) strcpy(v -> left_type, left_type);
        if (!right_null) strcpy(v -> right_type, right_type);
        strcpy(v -> return_type, return_type);
        strcpy(v -> signature, signature);

        HASH_ADD_STR(table -> map, signature, v);
    }
    else
    {
        // todo error and overload
    }

    free(signature);
}

void operation_add_native(struct operation_table* table, const char* left_type,  const char* op,
                          const char* c_format, const char* right_type, const char* return_type)
{
    char* signature = utils_new_string(256);
    operation_get_name(left_type, op, right_type, signature);

    struct operation* v = NULL;

    HASH_FIND_STR(table -> map, signature, v);

    if (v == NULL)
    {
        v = (struct operation*)(malloc(sizeof(struct operation)));
        const bool left_null = left_type == NULL;
        const bool right_null = right_type == NULL;
        const bool c_format_null = c_format == NULL;

        v -> op = utils_new_string(64);
        v -> left_type = left_null ? NULL : utils_new_string(32);
        v -> right_type = right_null ? NULL : utils_new_string(32);
        v -> return_type = utils_new_string(32);
        v -> signature = utils_new_string(256);
        v -> c_format = c_format_null ? NULL : utils_new_string(64);

        if ((!left_null && right_null) || (left_null && !right_null))
        {
            v -> arity = 1;
            v -> position = left_null ? PREFIX : POSITFIX;
        }
        else
            v -> arity = 2;

        strcpy(v -> op, op);
        if (!c_format_null) strcpy(v -> c_format, c_format);
        if (!left_null) strcpy(v -> left_type, left_type);
        if (!right_null) strcpy(v -> right_type, right_type);
        strcpy(v -> return_type, return_type);
        strcpy(v -> signature, signature);

        HASH_ADD_STR(table -> map, signature, v);
    }
    else
    {
        // todo error and overload
    }

    free(signature);
}

void operation_clear(struct operation_table* table)
{
    struct operation* v;
    struct operation* temp;

    HASH_ITER(hh, table -> map, v, temp)
    {
        HASH_DEL(table -> map, v);
        
        free(v -> op);
        try_free(v -> left_type);
        try_free(v -> right_type);
        try_free(v -> c_format);
        free(v -> signature);
        free(v -> return_type);
        free(v);
    }

    table -> map = NULL;
}

static const uint8_t X_NUMERIC_TYPES_LENGTH = 14;
static const char* X_NUMERIC_TYPES[14] =
{
    "bool", "char", "uchar", "byte", "ubyte",
    "short", "ushort", "int", "uint", // int index = 7
    "long", "ulong", // long index = 9
    "float", "double", "float128"
};

static const uint8_t X_INTEGER_TYPES_LENGTH = 11;
static const char* X_INTEGER_TYPES[11] =
{
    "bool", "char", "uchar", "byte", "ubyte",
    "short", "ushort", "int", "uint", // int index = 7
    "long", "ulong" // long index = 9
};

inline static int max(int a, int b)
{
    return a > b ? a : b;
}

inline static void format_dual_arity(const char* op, char* c_format)
{
    sprintf(c_format, "(%%s %s %%s)", op);
}

inline static void format_prefix(const char* op, char* c_format)
{
    sprintf(c_format, "(%s%%s)", op);
}

inline static void format_postfix(const char* op, char* c_format)
{
    sprintf(c_format, "(%%s%s)", op);
}

inline static void format_cast(const char* op, char* c_format)
{
    sprintf(c_format, "((%s)%%s)", op);
}

static void operation_add_numeric_compare(struct operation_table* table, const char* op, char* buffer1, char* buffer2, char* op_buffer)
{
    for (uint8_t i = 0; i < X_NUMERIC_TYPES_LENGTH; i++)
    {
        for (uint8_t j = 0; j < X_NUMERIC_TYPES_LENGTH; j++)
        {
            strcpy(buffer1, X_NUMERIC_TYPES[i]);
            strcpy(buffer2, X_NUMERIC_TYPES[j]);
            format_dual_arity(op, op_buffer);
            operation_add_native(table, buffer1, op, op_buffer, buffer2, "bool");
        }
    }
}

static void operation_add_integer_shift(struct operation_table* table, const char* op, char* buffer1, char* buffer2, char* op_buffer)
{
    for (uint8_t i = 0; i < X_INTEGER_TYPES_LENGTH; i++)
    {
        for (uint8_t j = 0; j < X_INTEGER_TYPES_LENGTH; j++)
        {
            strcpy(buffer1, X_INTEGER_TYPES[i]);
            strcpy(buffer2, X_INTEGER_TYPES[j]);
            format_dual_arity(op, op_buffer);

            if (i <= 7 && j <= 7)
                operation_add_native(table, buffer1, op, op_buffer, buffer2, "int");
            else
                operation_add_native(table, buffer1, op, op_buffer, buffer2, X_INTEGER_TYPES[max(i, j)]);
        }
    }
}

static void operation_add_dual_bit(struct operation_table* table, const char* op, char* buffer1, char* buffer2, char* op_buffer)
{
    const bool is_xnor = strcmp(op, "!^") == 0;

    for (uint8_t i = 0; i < X_INTEGER_TYPES_LENGTH; i++)
    {
        for (uint8_t j = 0; j < X_INTEGER_TYPES_LENGTH; j++)
        {
            strcpy(buffer1, X_INTEGER_TYPES[i]);
            strcpy(buffer2, X_INTEGER_TYPES[j]);

            if (i == 0 && j == 0)
            {
                if (strcmp(op, "|") == 0)
                {
                    format_dual_arity("||", op_buffer);
                    operation_add_native(table, buffer1, op, op_buffer, buffer2, "bool");
                }
                else if (strcmp(op, "&") == 0)
                {
                    format_dual_arity("&&", op_buffer);
                    operation_add_native(table, buffer1, op, op_buffer, buffer2, "bool");
                }
                else
                {
                    format_dual_arity(op, op_buffer);
                    operation_add_native(table, buffer1, op, is_xnor ? "(!(%%s ^ %%s))" : op_buffer, buffer2, "bool");
                }
            }
            else
            {
                format_dual_arity(op, op_buffer);

                if (i <= 7 && j <= 7)
                    operation_add_native(table, buffer1, op, is_xnor ? "(~(%%s ^ %%s))" : op_buffer, buffer2, "int");
                else
                    operation_add_native(table, buffer1, op, is_xnor ? "(~(%%s ^ %%s))" : op_buffer, buffer2, X_INTEGER_TYPES[max(i, j)]);
            }
        }
    }
}

static void operation_add_arithmetic(struct operation_table* table, const char* op, char* buffer1, char* buffer2, char* op_buffer)
{
    const bool is_mod = strcmp(op, "%%") == 0;
    const bool is_pow = strcmp(op, "**") == 0;

    for (uint8_t i = 0; i < X_NUMERIC_TYPES_LENGTH; i++)
    {
        for (uint8_t j = 0; j < X_NUMERIC_TYPES_LENGTH; j++)
        {
            strcpy(buffer1, X_NUMERIC_TYPES[i]);
            strcpy(buffer2, X_NUMERIC_TYPES[j]);
            format_dual_arity(op, op_buffer);

            if (is_pow)
            {
                if (i <= 12 && j <= 12)
                    operation_add_native(table, buffer1, op, "(pow(%s, %s))", buffer2, "double");
                else
                    operation_add_native(table, buffer1, op, "(powl(%s, %s))", buffer2, "float128");
            }
            else if (is_mod)
            {
                if (i <= 10 && j <= 10)
                    operation_add_native(table, buffer1, op, op_buffer, buffer2, "int");
            }
            else
            {
                if (i <= 7 && j <= 7)
                    operation_add_native(table, buffer1, op, op_buffer, buffer2, "int");
                else
                    operation_add_native(table, buffer1, op, op_buffer, buffer2, X_NUMERIC_TYPES[max(i, j)]);
            }
        }
    }
}

static void operation_add_prefix(struct operation_table* table, const char* op, char* buffer1, char* buffer2, char* op_buffer)
{
    const bool reverse_bit = strcmp(op, "!") == 0;

    for (uint8_t i = 0; i < X_NUMERIC_TYPES_LENGTH; i++)
    {
        strcpy(buffer2, X_NUMERIC_TYPES[i]);
        format_prefix(op, op_buffer);

        if (i == 0 && reverse_bit) // bool and reverse bit
            operation_add_native(table, NULL, op, op_buffer, buffer2, "bool");
        else if (i <= 10) // reverse
        {
            format_prefix(reverse_bit ? "~" : op, op_buffer);
            operation_add_native(table, NULL, op, op_buffer, buffer2, buffer2);
        }
        else if (i >= 11 && !reverse_bit)
            operation_add_native(table, NULL, op, op_buffer, buffer2, buffer2);
    }
}

static void operation_add_postfix(struct operation_table* table, const char* op, char* buffer1, char* buffer2, char* op_buffer)
{
    for (uint8_t i = 0; i < X_NUMERIC_TYPES_LENGTH; i++)
    {
        strcpy(buffer1, X_NUMERIC_TYPES[i]);
        format_postfix(op, op_buffer);
        operation_add_native(table, buffer1, op, op_buffer, NULL, buffer1);
    }
}

void operation_add_default(struct operation_table* dst_table)
{
    char* buffer1 = utils_new_string(32);
    char* buffer2 = utils_new_string(32);
    char* buffer3 = utils_new_string(32);

    // for (==, !=, >, <, >=, <=) 
    operation_add_numeric_compare(dst_table,  "==", buffer1, buffer2, buffer3);
    operation_add_numeric_compare(dst_table,  "!=", buffer1, buffer2, buffer3);
    operation_add_numeric_compare(dst_table,  ">", buffer1, buffer2, buffer3);
    operation_add_numeric_compare(dst_table,  "<", buffer1, buffer2, buffer3);
    operation_add_numeric_compare(dst_table,  ">=", buffer1, buffer2, buffer3);
    operation_add_numeric_compare(dst_table,  "<=", buffer1, buffer2, buffer3);

    // for (<<, >>)
    operation_add_integer_shift(dst_table , "<<", buffer1, buffer2, buffer3);
    operation_add_integer_shift(dst_table , ">>", buffer1, buffer2, buffer3);

    // for (|, ^, !^, &)
    operation_add_dual_bit(dst_table , "|", buffer1, buffer2, buffer3);
    operation_add_dual_bit(dst_table , "^", buffer1, buffer2, buffer3);
    operation_add_dual_bit(dst_table , "!^", buffer1, buffer2, buffer3);
    operation_add_dual_bit(dst_table , "&", buffer1, buffer2, buffer3);

    // for (+, -, *, /, %)
    operation_add_arithmetic(dst_table , "+", buffer1, buffer2, buffer3);
    operation_add_arithmetic(dst_table , "-", buffer1, buffer2, buffer3);
    operation_add_arithmetic(dst_table , "*", buffer1, buffer2, buffer3);
    operation_add_arithmetic(dst_table , "/", buffer1, buffer2, buffer3);
    operation_add_arithmetic(dst_table , "%%", buffer1, buffer2, buffer3);
    operation_add_arithmetic(dst_table , "**", buffer1, buffer2, buffer3);

    // for (<+>, <->, !, ++, --)
    operation_add_prefix(dst_table , "+", buffer1, buffer2, buffer3);
    operation_add_prefix(dst_table , "-", buffer1, buffer2, buffer3);
    operation_add_prefix(dst_table , "!", buffer1, buffer2, buffer3);
    operation_add_prefix(dst_table , "++", buffer1, buffer2, buffer3);
    operation_add_prefix(dst_table , "--", buffer1, buffer2, buffer3);

    // for (++, --)
    operation_add_postfix(dst_table , "++", buffer1, buffer2, buffer3);
    operation_add_postfix(dst_table , "--", buffer1, buffer2, buffer3);

    for (uint8_t i = 0; i < X_NUMERIC_TYPES_LENGTH; i++)
    {
        type_tran_to_ctype(X_NUMERIC_TYPES[i], buffer2);
        format_cast(buffer2, buffer3);

        for (uint8_t j = 0; j < X_NUMERIC_TYPES_LENGTH; j++)
            operation_add_native(dst_table, NULL, X_NUMERIC_TYPES[i], buffer3, X_NUMERIC_TYPES[j], X_NUMERIC_TYPES[i]);
    }

    free(buffer1);
    free(buffer2);
    free(buffer3);
}

void operation_get(const struct operation_table* table, const char* left, const char* op, const char* right, struct operation* dst)
{
    char* signature = utils_new_string(256);
    operation_get_name(left, op, right, signature);
    struct operation* v = NULL;
    HASH_FIND_STR(table -> map, signature, v);
    *dst = *v;
    free(signature);
}

bool operation_contain(const struct operation_table* table, const char* left, const char* op, const char* right)
{
    char* signature = utils_new_string(256);
    operation_get_name(left, op, right, signature);
    struct operation* v = NULL;
    HASH_FIND_STR(table -> map, signature, v);
    free(signature);
    return v != NULL;
}