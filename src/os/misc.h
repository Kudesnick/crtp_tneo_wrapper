
#include <stdint.h>

#include "printf.h"

#define BRK for(;;)

#define STRINGIFY(x) #x
#define STR(x) STRINGIFY(x)
#define printerr(_s, ...) printf("\033[31mError:\033[0m '" __FILE__ "'[" STR(__LINE__) "] : " _s __VA_OPT__(,) __VA_ARGS__)
#define PRINTFAULT(...) printerr(__VA_ARGS__); BRK

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

constexpr unsigned long long operator "" _min(unsigned long long min)
{
    return static_cast<unsigned long long>(min * 1000U * 1000U);
}

#endif // __cplusplus
