
#pragma once

#include <stdint.h>
#include "tn.h"

extern"C" void tn_tick_int_processing(void);

extern"C" void tn_arch_int_dis(void);
extern"C" void tn_sys_start(uint32_t *, unsigned int, void(*)(void));

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

enum class rc: int8_t
{
   ok          =   0,
   timeout     =  -1,
   overflow    =  -2,
   wcontext    =  -3,
   wstate      =  -4,
   wparam      =  -5,
   illegal_use =  -6,
   invalid_obj =  -7,
   deleted     =  -8,
   forced      =  -9,
   internal    = -10
};

template <uint32_t sz>
struct stack
{
    static_assert(sz % sizeof(uint64_t) == 0, "");
    static_assert(sz  / sizeof(uint32_t) >= TN_MIN_STACK_SIZE, "");
    uint64_t stack_ptr[sz / sizeof(uint64_t)];
    static inline constexpr auto stack_size = sz;
};

template <class T> constexpr void(*member_to_func(void(T::*_member)(void)))(void *)
{
    const union
    {
        void(T::*member)(void);
        void(*func)(void *);
    } &ptr = {.member = _member};
    
    return ptr.func;
}

class task_base
{
protected:
    TN_Task task_;

    rc sleep(const uint32_t _tick);
    rc yield(void);
    void exit(void);
    void self_destructor(void);

public:
    enum class state: int8_t
    {
        none     = 0,
        runnable = (1 << 0),
        wait     = (1 << 1),
        suspend  = (1 << 2),
        waitsusp = (wait | suspend),
        dormant  = (1 << 3),
        err      = -1
        
    };

    rc suspend(void);
    rc resume(void);
    rc terminate(void);
    state state_get(void);
    rc change_priority(const priority _priority);
    rc wakeup(void);
    rc activate(void);
    rc release_wait(void);
    bool is_task_context(void);
    ~task_base();
};


class kernel_base
{
public:
    static void tick(void)
    {
        tn_tick_int_processing();
    }
    static rc tslice_set(const priority _priority, const int32_t _ticks)
    {
        return static_cast<rc>(tn_sys_tslice_set(static_cast<int>(_priority), _ticks));
    }
};


template <class T, uint32_t _stack_size, os::priority _priority = os::priority::normal>
class task : public task_base, private stack<_stack_size>
{
public:
    task(const char *const _name = nullptr)
    {
        tn_task_create_wname(
            &task_,
            member_to_func(&T::task_func),
            static_cast<int>(_priority),
            reinterpret_cast<TN_UWord *>(this->stack_ptr),
            this->stack_size / sizeof(uint32_t),
            this,
            _name
            );
    }
};


template <class T, uint32_t _stack_size>
class kernel: public task<T, _stack_size, priority::idle>, public kernel_base
{
public:
    kernel(uint32_t *const _sys_stack_ptr, const uint32_t _sys_stack_size):
        task<T, _stack_size, priority::idle>("idle")
    {
        tn_arch_int_dis();
        static_cast<T*>(this)->hw_init();
        tn_sys_start(
            _sys_stack_ptr,
            _sys_stack_size / 4,
            T::sw_init
            );
    }
};

} // namespace os
