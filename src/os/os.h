
#pragma once

#include <stdint.h>
#include "tn.h"
#include "bsp.h"

namespace os
{

enum class priority: uint8_t
{
    idle     = 4,
    low      = 3,
    normal   = 2,
    high     = 1,
    relatime = 0
};

inline static constexpr uint32_t MIN_STACK_SIZE = TN_MIN_STACK_SIZE;

template <uint32_t sz> struct stack
{
    static_assert(sz % sizeof(uint64_t) == 0, "");
    static_assert(sz  / sizeof(uint32_t) >= TN_MIN_STACK_SIZE, "");
private:
    uint64_t dword[sz / sizeof(uint64_t)];
};

class task
{
protected:
    TN_Task task_;

    static void sleep(const uint32_t _tick)
    {
        tn_task_sleep(_tick);
    }

public:
    template<class T> task(T *const &_base, const priority _priority = priority::normal, const char *const _name = nullptr)
    {
        tn_task_create_wname(
            &task_,
            _base->task_func,
            static_cast<int>(_priority),
            reinterpret_cast<TN_UWord *>(&(_base->stack)),
            sizeof(_base->stack) / sizeof(uint32_t),
            _base,
            _name
            );
    }
};

class kernel: public task
{
public:
    static inline void tick(void)
    {
        tn_tick_int_processing();
    }

    template<class T> kernel(T *const &_base):
        task(_base, priority::idle, "idle")
    {
        tn_arch_int_dis();
        _base->init_hw();
        tn_sys_start(
            csp::stack_ptr,
            csp::stack_size / 4,
            _base->init_sw
            );
    }
};

} // namespace os
