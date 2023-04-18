
#pragma once

#include <stdint.h>
#include "tn.h"

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

inline static constexpr uint32_t MIN_STACK_SIZE = TN_MIN_STACK_SIZE;

template <uint32_t sz>
struct stack
{
    static_assert(sz % sizeof(uint64_t) == 0, "");
    static_assert(sz  / sizeof(uint32_t) >= TN_MIN_STACK_SIZE, "");
    uint64_t arr[sz / sizeof(uint64_t)];
    static constexpr auto size = sz;
};

class task_base
{
protected:
    TN_Task task_;

    rc sleep(const uint32_t _tick)
    {
        return static_cast<rc>(tn_task_sleep(_tick));
    }
    
    void exit(void)
    {
        tn_task_exit(TN_TASK_EXIT_OPT_NO_DELETE);
    }

    void self_destructor(void)
    {
        tn_task_exit(TN_TASK_EXIT_OPT_DELETE);
    }

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

    rc suspend(void)
    {
        return static_cast<rc>(tn_task_suspend(&task_));
    }

    rc resume(void)
    {
        return static_cast<rc>(tn_task_resume(&task_));
    }

    rc terminate(void)
    {
        return static_cast<rc>(tn_task_terminate(&task_));
    }

    state state_get(void)
    {
        auto r_state = static_cast<enum TN_TaskState>(state::err);

        tn_task_state_get(&task_, &r_state);
        return static_cast<state>(r_state);
    }

    rc change_priority(const priority _priority)
    {
        return static_cast<rc>(tn_task_change_priority(&task_, static_cast<int>(_priority)));
    }

    rc wakeup(void)
    {
        return static_cast<rc>(
            tn_is_task_context() ? tn_task_wakeup(&task_) : tn_task_iwakeup(&task_)
            );
    }

    rc activate(void)
    {
        return static_cast<rc>(
            tn_is_task_context() ? tn_task_activate(&task_) : tn_task_iactivate(&task_)
            );
    }

    rc release_wait(void)
    {
        return static_cast<rc>(
            tn_is_task_context() ? tn_task_release_wait(&task_) : tn_task_irelease_wait(&task_)
            );
    }

    bool is_task_context(void)
    {
        return tn_is_task_context();
    }

    ~task_base(void)
    {
        tn_task_delete(&task_);
    }
};

template <class T, uint32_t _stack_size, os::priority _priority = os::priority::normal>
    class task : public task_base, private stack<_stack_size>
{
public:
    constexpr task(const char *const _name = nullptr)
    {
        tn_task_create_wname(
            &task_,
            [](void *_T){static_cast<T*>(_T)->task_func();},
            static_cast<int>(_priority),
            reinterpret_cast<TN_UWord *>(&this->arr),
            this->size / sizeof(uint32_t),
            this,
            _name
            );
    }
};

template <class T, uint32_t _stck_size>
class kernel: public task<T, _stck_size, priority::idle>
{
public:
    static void tick(void)
    {
        tn_tick_int_processing();
    }

    constexpr kernel(uint32_t *const _stack_ptr, uint32_t _stack_size):
        task<T, _stck_size, priority::idle>("idle")
    {
        tn_arch_int_dis();
        T::hw_init();
        tn_sys_start(
            _stack_ptr,
            _stack_size / 4,
            T::sw_init
            );
    }
};

} // namespace os
