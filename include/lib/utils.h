#ifndef _LIB_UTILS_
#define _LIB_UTILS_

#include <stdbool.h>
#include <stdint.h>

#define VALID_CHARACTERS "0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define FIRST_CHARACTERS "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"

void utils_random_name(char* dst, uint8_t len);
void utils_substring(const char* src, uint16_t from_index, uint16_t to_index, char* dst);
bool utils_code_contain(const char* std_code, const char* str);
bool utils_string_contain(const char* src, const char* str);
uint64_t utils_string_indexof(const char* src, const char* str);
uint64_t utils_code_indexof(const char* src, const char* str);
char* utils_new_string(uint64_t length);

bool utils_is_digit(char c);
bool utils_is_uppercase(char c);
bool utils_is_lowercase(char c);
bool utils_is_letter(char c);
bool utils_is_space(char c);
bool utils_is_escape_sequences(char c);
bool utils_is_number(const char* x_code);
bool utils_is_string(const char* x_code);

bool utils_cmd(const char* command);

void utils_simplify_code(const char* x_code, char* dst);

int printf_err(const char* fmt, ...);

#endif
