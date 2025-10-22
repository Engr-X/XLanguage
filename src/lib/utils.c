#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include "lib/utils.h"

void utils_random_name(char* dst, uint8_t len)
{
    if (len <= 0)
        return;

    const char* first_chars = FIRST_CHARACTERS;
    const uint8_t first_len = strlen(first_chars) - 1;
    dst[0] = first_chars[rand() % first_len];

    const char* valid_chars = VALID_CHARACTERS;
    const uint8_t valid_len = strlen(valid_chars) - 1;

    for (uint8_t i = 1; i < len; i++)
        dst[i] = valid_chars[rand() % valid_len];

    dst[len + 1] = '\0';
}

void utils_substring(const char* src, uint16_t from_index, uint16_t to_index, char* dst)
{
    uint16_t len = to_index - from_index;
    strncpy(dst, src + from_index, len);
    dst[len] = '\0';
}

bool utils_string_contain(const char* src, const char* str)
{
    const uint64_t strlen1 = strlen(src);
    const uint64_t strlen2 = strlen(str);

    for (uint64_t i = 0; i < strlen1; i++)
    {
        if (src[i] == str[0])
        {
            if (i + strlen2 > strlen1)
                break;

            bool flag = true;

            for (uint64_t j = 1; j < strlen2; j++)
            {
                if (src[i + j] != str[j])
                {
                    flag = false;
                    break;
                }
            }

            if (flag)
                return true;
        }
    }

    return false;
}

bool utils_code_contain(const char* std_code, const char* str)
{
    const uint64_t len_code = strlen(std_code);
    const uint64_t len_str  = strlen(str);

    for (uint64_t i = 0; i < len_code; i++)
    {
        if (std_code[i] == '"')
        {
            i++;

            while (i < len_code && std_code[i] != '"')
                i++;

            continue;
        }

        if (std_code[i] == '\'')
        {
            i++;

            while (i < len_code && std_code[i] != '\'')
                i++;

            continue;
        }

        if (std_code[i] == str[0])
        {
            if (i + len_str > len_code)
                break;

            bool match = true;

            for (uint64_t j = 1; j < len_str; j++)
            {
                if (std_code[i + j] != str[j])
                {
                    match = false;
                    break; 
                }
            }

            if (match)
                return true;
        }
    }

    return false;
}


uint64_t utils_string_indexof(const char* src, const char* str)
{
    const uint64_t strlen1 = strlen(src);
    const uint64_t strlen2 = strlen(str);

    for (uint64_t i = 0; i < strlen1; i++)
    {
        if (src[i] == str[0])
        {
            if (i + strlen2 > strlen1)
                break;

            bool match = true;

            for (uint64_t j = 1; j < strlen2; j++)
            {
                if (src[i + j] != str[j])
                {
                    match = false;
                    break;
                }
            }

            if (match)
                return i;
        }
    }

    return -1;
}


uint64_t utils_code_indexof(const char* std_code, const char* str)
{
    const uint64_t len_code = strlen(std_code);
    const uint64_t len_str  = strlen(str);

    for (uint64_t i = 0; i < len_code; i++)
    {

        if (std_code[i] == '"')
        {
            i++;

            while (i < len_code && std_code[i] != '"')
                i++;

            continue;
        }

        if (std_code[i] == '\'')
        {
            i++;

            while (i < len_code && std_code[i] != '\'')
                i++;
                
            continue;
        }

        if (std_code[i] == str[0])
        {
            if (i + len_str > len_code)
                return -1;

            bool match = true;

            for (uint64_t j = 1; j < len_str; j++)
            {
                if (std_code[i + j] != str[j])
                {
                    match = false;
                    break;
                }
            }

            if (match)
                return i;
        }
    }

    return -1;
}

char* utils_new_string(uint64_t length)
{
    length++;
    char* string = (char*)(malloc(length * sizeof(char)));
    *string = '\0';
    return string;
}

void utils_simplify_code(const char* x_code, char* dst)
{
    const char* src = x_code;
    char* out = dst;

    while (*src)
    {
        if (*src == '"' || *src == '\'')
        {
            const char quote = *src++;
            *out++ = quote;

            while (*src && !(*src == quote && *(src - 1) != '\\'))
                *out++ = *src++;

            if (*src)
                *out++ = *src++;
        }
        else if (!utils_is_space(*src))
            *out++ = *src++;
        else
            src++;
    }

    *out = '\0';
}

bool utils_cmd(const char* command)
{
    int ret = system(command);
    return ret == 0;
}

bool utils_is_digit(char c)
{
    return ('0' <= c && c <= '9') || c == '-' || c == '+' || c == '.';
}

bool utils_is_uppercase(char c)
{
    return 'A' <= c && c <= 'Z';
}

bool utils_is_lowercase(char c)
{
    return 'a' <= c && c <= 'z';
}

bool utils_is_letter(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool utils_is_escape_sequences(char c)
{
    return c == '\n' || c == '\t' || c == '\r' || c == '\b' || c == '\f' || c == '\v';
}

bool utils_is_space(char c)
{
    return c == ' ' || utils_is_escape_sequences(c);
}

bool utils_is_number(const char* xcode)
{
    if (!xcode || *xcode == '\0')
        return false;

    if (*xcode == '-' || *xcode == '+')
        xcode++;

    if (*xcode == '.')
        xcode++;
   
    return utils_is_digit(*xcode);
}

bool utils_is_char(const char* xcode)
{
    return *xcode == '\'';
}

bool utils_is_string(const char* xcode)
{
    return *xcode == '\"';
}


int printf_err(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "\033[31m");
    int result = vfprintf(stderr, fmt, args);
    fprintf(stderr, "\033[0m");

    va_end(args);
    return result;
}