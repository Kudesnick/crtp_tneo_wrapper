
#include <stdint.h>
#include "tn.h"

namespace os
{
    
enum class priority: uint8_t
{
    idle     = 0,
    low      = 1,
    normal   = 2,
    high     = 3,
    relatime = 4
};

inline void hardfault(void)
{
    volatile static uint32_t *i = nullptr;
    *i++;
}

template <uint32_t sz> union stack
{
    static_assert(sz % sizeof(uint64_t) == 0, "");
    static_assert(sz  / sizeof(uint32_t) >= TN_MIN_STACK_SIZE, "");
    uint64_t dword[sz / sizeof(uint64_t)];
    uint32_t word[sz / sizeof(uint32_t)];
    static inline constexpr uint32_t size = sz / sizeof(uint32_t);
};

template <typename T> class kernel
{
public:
    static inline void tick(void)
    {
        tn_tick_int_processing();
    }

    kernel(void)
    {
        tn_arch_int_dis();
        T *const &base = static_cast<T*>(this);
        base->init_hw();
        tn_sys_start(
            base->idle_stack.word,
            base->idle_stack.size,
            base->int_stack.word,
            base->int_stack.size,
            base->idle_task,
            base->init_sw
            );
    }
};

} // namespace os
