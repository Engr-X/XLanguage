#include <math.h>
#include <time.h>

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

long long __native_xl_get_time()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

#endif