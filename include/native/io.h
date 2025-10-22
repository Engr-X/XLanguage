#ifndef _X_NATIVE_IO_H_
#define _X_NATIVE_IO_H_

void __native_xl_newline();

void __native_xl_add(int a, int b);
void __native_xl_add3(int a, int b, int c);

void __native_xl_print_bool(bool value);
void __native_xl_print_char(char value);
void __native_xl_print_byte(char value);
void __native_xl_print_short(short value);
void __native_xl_print_int(int value);
void __native_xl_print_long(long long value);
void __native_xl_print_float(float value);
void __native_xl_print_double(double value);
void __native_xl_print_str(const char* value);

void __native_xl_println_bool(bool value);
void __native_xl_println_char(char value);
void __native_xl_println_byte(char value);
void __native_xl_println_short(short value);
void __native_xl_println_int(int value);
void __native_xl_println_long(long long value);
void __native_xl_println_float(float value);
void __native_xl_println_double(double value);
void __native_xl_println_str(const char* value);

#endif