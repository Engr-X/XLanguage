#include <math.h>

#ifndef _BSD_SOURCE
#define _BSD_SOURCE

int __native_xl_max_int(int a, int b)
{
    return a > b ? a : b;
}

int __native_xl_min_int(int a, int b)
{
    return a < b ? a : b;
}

double __native_xl_max_double(double a, double b)
{
    return a > b ? a : b;
}

double __native_xl_min_double(double a, double b)
{
    return a < b ? a : b;
}

double __native_xl_sum_3double(double a, double b, double c)
{
    return a + b + c;
}

double __native_xl_pi()
{
    return M_PI;
}

#endif