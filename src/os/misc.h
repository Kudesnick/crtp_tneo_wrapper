
#include <stdio.h>
#include <stdint.h>

#define STRINGIFY(x) #x
#define STR(x) STRINGIFY(x)
#define printerr(_s, ...) fprintf(stderr, "\033[31mError:\033[0m '" __FILE__ "'[" STR(__LINE__) "] : " _s __VA_OPT__(,) __VA_ARGS__)

#define U32 "%#010x"

#ifdef __cplusplus

constexpr unsigned long long operator "" _KiB(unsigned long long bytes)
{
    return static_cast<unsigned long long>(bytes * 1024U);
}

constexpr unsigned long long operator "" _sec(long double sec)
{
    return static_cast<unsigned long long>(sec * 1000U);
}

constexpr unsigned long long operator "" _sec(unsigned long long sec)
{
    return static_cast<unsigned long long>(sec * 1000U);
}

#endif // __cplusplus
