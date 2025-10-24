#include <stdio.h>
#include <stdbool.h>

#include "native/io.h"

inline void __native_xl_newline()
{
    puts("");
}

inline void __native_xl_add(int a, int b)
{
    printf("%d + %d = %d\n", a, b, a + b);
}

inline void __native_xl_add3(int a, int b, int c)
{
    printf("%d + %d + %d = %d\n", a, b, c, a + b + c);
}

inline void __native_xl_print_bool(bool value)
{
    printf("%s", value ? "true" : "false");
}

inline void __native_xl_print_char(char value)
{
    putchar(value);
}

inline void __native_xl_print_byte(char value)
{
    printf("%d", value);
}

inline void __native_xl_print_short(short value)
{
    printf("%d", value);
}

inline void __native_xl_print_int(int value)
{
    printf("%d", value);
}

inline void __native_xl_print_long(long long value)
{
    printf("%lld", value);
}

inline void __native_xl_print_float(float value)
{
    printf("%.4f", value);
}

inline void __native_xl_print_double(double value)
{
    printf("%lf", value);
}

inline void __native_xl_print_str(const char* value)
{
    printf("%s", value);
}


inline void __native_xl_println_bool(bool value)
{
    puts(value ? "true" : "false");
}

inline void __native_xl_println_char(char value)
{
    printf("%c\n", value);
}

inline void __native_xl_println_byte(char value)
{
    printf("%d\n", value);
}

inline void __native_xl_println_short(short value)
{
    printf("%d\n", value);
}

inline void __native_xl_println_int(int value)
{
    printf("%d\n", value);
}

inline void __native_xl_println_long(long long value)
{
    printf("%lld\n", value);
}

inline void __native_xl_println_float(float value)
{
    printf("%.4f\n", value);
}

inline void __native_xl_println_double(double value)
{
    printf("%lf\n", value);
}

inline void __native_xl_println_str(const char* value)
{
    puts(value);
}