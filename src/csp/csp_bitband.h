#pragma once

namespace csp
{

namespace bb
{

constexpr static inline uint8_t mask_to_bit(const uint32_t _mask)
{
#if (0)
    return (_mask & 1 <<  0) ?  0 : 1 + mask_to_bit(_mask >> 1);
#else
    return (_mask & 1 <<  0) ?  0 :
           (_mask & 1 <<  1) ?  1 :
           (_mask & 1 <<  2) ?  2 :
           (_mask & 1 <<  3) ?  3 :
           (_mask & 1 <<  4) ?  4 :
           (_mask & 1 <<  5) ?  5 :
           (_mask & 1 <<  6) ?  6 :
           (_mask & 1 <<  7) ?  7 :
           (_mask & 1 <<  8) ?  8 :
           (_mask & 1 <<  9) ?  9 :
           (_mask & 1 << 10) ? 10 :
           (_mask & 1 << 11) ? 11 :
           (_mask & 1 << 12) ? 12 :
           (_mask & 1 << 13) ? 13 :
           (_mask & 1 << 14) ? 14 :
           (_mask & 1 << 15) ? 15 :
           (_mask & 1 << 16) ? 16 :
           (_mask & 1 << 17) ? 17 :
           (_mask & 1 << 18) ? 18 :
           (_mask & 1 << 19) ? 19 :
           (_mask & 1 << 20) ? 20 :
           (_mask & 1 << 21) ? 21 :
           (_mask & 1 << 22) ? 22 :
           (_mask & 1 << 23) ? 23 :
           (_mask & 1 << 24) ? 24 :
           (_mask & 1 << 25) ? 25 :
           (_mask & 1 << 26) ? 26 :
           (_mask & 1 << 27) ? 27 :
           (_mask & 1 << 28) ? 28 :
           (_mask & 1 << 29) ? 29 :
           (_mask & 1 << 30) ? 30 :
           (_mask & 1 << 31) ? 31 : 0;
#endif
}

constexpr static inline uint32_t addr_calc(const uint32_t _addr, const uint8_t _bit)
{
    return (_addr & 0xF0000000) + ((_addr & 0x00FFFFFF) << 5) + 0x02000000 + (_bit << 2);
}
  
static inline uint32_t & bb_bit(volatile uint32_t & _addr, const uint8_t _bit)
{
    return *reinterpret_cast<uint32_t *const>(addr_calc(reinterpret_cast<const uint32_t>(&_addr), _bit));
}

static inline void set(volatile uint32_t & _addr, const uint32_t _mask)
{
    bb_bit(_addr, mask_to_bit(_mask)) = 1;
}

static inline void clr(volatile uint32_t & _addr, const uint32_t _mask)
{
    bb_bit(_addr, mask_to_bit(_mask)) = 0;
}

static inline uint32_t get(volatile uint32_t & _addr, const uint32_t _mask)
{
    return bb_bit(_addr, mask_to_bit(_mask));
}


}; // namespace bb

}; // namespace csp
