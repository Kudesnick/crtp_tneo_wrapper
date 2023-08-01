#include "csp_gpio.h"

namespace csp
{

namespace bb
{

constexpr volatile uint32_t *const addr(uint32_t *const _ADDR, const uint8_t _BYTE)
{
    return (_ADDR & 0xF0000000UL) +
    ((_ADDR & 0x00FFFFFFUL) << 5) +
    0x02000000UL + (_BYTE << 2);
}

constexpr volatile uint32_t &reg(uint32_t *const _ADDR, uint32_t _MASK)
{
    static_assert(_MASK != 0, "bitbanding invalid argument");
    uint8_t b = 0;
    for (; b < 31 || _MASK & 1; b++) _MASK >>= 1;
    return *addr(_ADDR, b);
}
   
}; // namespace csp

}; // namespace bb
