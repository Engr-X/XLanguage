#include <stdio.h>
#include <string.h>

#include "lib/utils.h"
#include "lib/core_types.h"
#include "core/type_convert.h"
#include "native/constants.h"

#define TYPE_COUNT 25

#define BOOL_TYPE "bool"
#define CHAR_TYPE "char"
#define STRING_TYPE "string"
#define FLOAT_TYPE "float"
#define DOUBLE_TYPE "double"
#define LONG_TYPE "long"
#define INT_TYPE "int"
#define UINT_TYPE "uint"

const char* X_BASIC_TYPES[25] =
{
    "bool", "char", "byte", "short", "int", "long", "float", "double", "string",            // index: 0 ~ 9
    "uchar", "ubyte", "ushort", "uint", "ulong",                                            // index: 9 ~ 14
    "int8", "int16", "int32", "int64", "float32", "float64", "float128",                    // index: 14 ~ 20
    "uint8", "uint16", "uint32", "uint64",                                                  // index: 20 ~ 23
};

const char* C_TYPES_NAMES[25] = 
{
    "char", "char", "char", "short", "int", "long long", "float", "double", "char*",
    "unsigned char", "unsigned char", "unsigned short", "unsigned int", "unsigned long long",
    "char", "short", "int", "long long", "float", "double", "long double",
    "unsigned char", "unsigned short", "unsigned int", "unsigned long long",
};

void type_tran_print()
{
    for (ubyte i = 0; i < TYPE_COUNT; i++)
        printf("X: %s, C: %s\n", X_BASIC_TYPES[i], C_TYPES_NAMES[i]);
}

void type_tran_to_ctype(const char* xtype, char* dst)
{
    for (uint8_t i = 0; i < TYPE_COUNT; i++)
    {
        if (strcmp(xtype, X_BASIC_TYPES[i]) == 0)
        {
            strcpy(dst, C_TYPES_NAMES[i]);
            return;
        }
    }

    strcpy(dst, "error");
}

bool type_is_basic_type(const char* xtype)
{
    for (uint8_t i = 0; i < TYPE_COUNT; i++)
    {
        if (strcmp(xtype, X_BASIC_TYPES[i]) == 0)
            return true;
    }

    return false;
}

bool type_tran_isbool(const char* xtype)
{
    return strcmp(xtype, FALSE_STRING) == 0 || strcmp(xtype, TRUE_STRING) == 0;
}

bool type_tran_is_cast(const char* xtype)
{
    if (*xtype != '(')
        return false;

    const char* p = xtype + 1;

    while (*p && *p != ')')
        p++;
   
    return *p == ')';
}

bool variable_is_constant(const char* x_code)
{
    if (utils_is_digit(*x_code) || ((*x_code == '-' || *x_code == '+') && utils_is_digit(x_code[1])) ||
        (*x_code == '.' && utils_is_digit(x_code[1])) ||
        ((*x_code == '-' || *x_code == '+') && x_code[1] == '.' && utils_is_digit(x_code[2])))
        return true;

    if (*x_code == '\'' || *x_code == '\"')
        return true;

    if (strcmp(x_code, TRUE_STRING) == 0 || strcmp(x_code, FALSE_STRING) == 0)
        return true;
    
    if (strcmp(x_code, NULL_STRING) == 0)
        return true;

    return false;
}

void variable_constant_type(const char* x_code, char* dst)
{
    if (strcmp(x_code, TRUE_STRING) == 0 || strcmp(x_code, FALSE_STRING) == 0)
    {
        strcpy(dst, BOOL_TYPE);
        return;
    }

    const char first = *x_code;
    const char last = x_code[strlen(x_code) - 1];

    if (first == '\'')
    {
        strcpy(dst, CHAR_TYPE);
        return;
    }

    if (first == '\"')
    {
        strcpy(dst, STRING_TYPE);
        return;
    }

    if (last == 'f' || last == 'F')
    {
        strcpy(dst, FLOAT_TYPE);
        return;
    }

    if (last == 'l' || last == 'L')
    {
        strcpy(dst, LONG_TYPE);
        return;
    }

    if (last == 'u' || last == 'U')
    {
        strcpy(dst, UINT_TYPE);
        return;
    }

    bool find_dot = false, contain_e = false;

    for (const char* p = x_code; *p; p++)
    {
        if (*p == '.')
            find_dot = true;
        
        if (*p == 'e' || *p == 'E')
            contain_e = true;

        if (find_dot && contain_e)
            break;
    }

    strcpy(dst, find_dot || contain_e ? DOUBLE_TYPE : INT_TYPE);
}

void variable_constant_tran(const char* x_code, char* dst)
{
    char* last = (char*)(x_code + strlen(x_code) - 1);

    if (strcmp(x_code, TRUE_STRING) == 0)
    {
        strcpy(dst, TRUE_VALUE_STRING);
        return;
    }

    if (strcmp(x_code, FALSE_STRING) == 0)
    {
        strcpy(dst, FALSE_VALUE_STRING);
        return;
    }

    if (strcmp(x_code, NULL_STRING) == 0)
    {
        strcpy(dst, NULL_VALUE_STRING);
        return;
    }

    if ((*last == 'l' || *last == 'L') && (*(last - 1) != 'l' || *(last - 1) != 'L'))
    {
        if (*last == 'l')
            *(last + 1) = 'l';

        if (*last == 'L')
            *(last + 1) = 'L';  
        
        *(last + 2) = '\0';
    }

    strcpy(dst, x_code);
}