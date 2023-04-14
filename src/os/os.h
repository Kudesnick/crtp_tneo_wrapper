
#pragma once

#include <stdint.h>
#include <memory>
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

class kernel
{
public:
    static inline void tick(void)
    {
        tn_tick_int_processing();
    }

    template<typename T> kernel(T *const &_base)
    {
        tn_arch_int_dis();
        _base->init_hw();
        tn_sys_start(
            reinterpret_cast<TN_UWord *>(std::addressof(_base->idle_stack)),
            sizeof(_base->idle_stack) / 4,
            csp::stack_ptr,
            csp::stack_size / 4,
            _base->init_sw,
            _base->idle_task
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
        tn_task_create_wname(
            &task_,
            [](void *_base){static_cast<T*>(_base)->task_func();},
            static_cast<int>(_priority),
            reinterpret_cast<TN_UWord *>(std::addressof(base->stack)),
            sizeof(base->stack) / sizeof(uint32_t),
            base,
            TN_TASK_CREATE_OPT_START,
            _name
            );
    }
};

} // namespace os
