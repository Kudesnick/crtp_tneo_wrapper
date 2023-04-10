
#include <stdint.h>
#include "tn.h"

extern "C" const uint32_t Image$$RW_STACK$$Base;
extern "C" const uint32_t Image$$RW_STACK$$Length;

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

inline static constexpr uint32_t MIN_STACK_SIZE = TN_MIN_STACK_SIZE;

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
//          base->int_stack.word,
//          base->int_stack.size,
            const_cast<uint32_t *>(&Image$$RW_STACK$$Base),
            (Image$$RW_STACK$$Length - reinterpret_cast<uint32_t>(&Image$$RW_STACK$$Base)) / sizeof(uint32_t),
            base->init_sw,
            base->idle_task
            );
    }
};

template<typename T> class task
{
protected:
    TN_Task task_;

    static void sleep(const uint32_t _tick)
    {
        tn_task_sleep(_tick);
    }

public:
    task(const char *const _name):
        task(priority::normal, _name)
        {}

    task(
        const priority _priority = priority::normal,
        const char *const _name = nullptr
        )
    {
        T *const &base = static_cast<T*>(this);
        if (tn_task_create_wname(
                &task_,
                [](void *_base){static_cast<T*>(_base)->task_func();},
                static_cast<int>(_priority),
                base->stack.word,
                base->stack.size,
                base,
                TN_TASK_CREATE_OPT_START,
                _name
                ) != TN_RC_OK)
        {
            hardfault();
        }
    }
};

} // namespace os
