
#pragma once

#include <stdint.h>


// extern"C" void tn_tick_int_processing(void);
// extern"C" void tn_arch_int_dis(void);
// extern"C" void tn_sys_start(uint32_t *, unsigned int, void(*)(void));

namespace os
{
    
namespace __tn
{
#include "tn.h"
}

//-- utils ----------------------------------------------------------------------------------------/

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

//-- common data from tn_common.h and tn_sys.h ----------------------------------------------------/

enum class rc: int8_t
{
   ok          =  __tn::TN_RC_OK         ,
   timeout     =  __tn::TN_RC_TIMEOUT    ,
   overflow    =  __tn::TN_RC_OVERFLOW   ,
   wcontext    =  __tn::TN_RC_WCONTEXT   ,
   wstate      =  __tn::TN_RC_WSTATE     ,
   wparam      =  __tn::TN_RC_WPARAM     ,
   illegal_use =  __tn::TN_RC_ILLEGAL_USE,
   invalid_obj =  __tn::TN_RC_INVALID_OBJ,
   deleted     =  __tn::TN_RC_DELETED    ,
   forced      =  __tn::TN_RC_FORCED     ,
   internal    =  __tn::TN_RC_INTERNAL   
};

enum class priority: uint8_t
{
    idle     = 4,
    low      = 3,
    normal   = 2,
    high     = 1,
    relatime = 0,
};

enum class sys_state: uint8_t
{
    noinit  = __tn::TN_STATE_FLAG__SYS_NOINIT,
    running = __tn::TN_STATE_FLAG__SYS_RUNNING,
    dedlock = __tn::TN_STATE_FLAG__DEADLOCK
};

enum class context: uint8_t
{
    none = __tn::TN_CONTEXT_NONE,
    task = __tn::TN_CONTEXT_TASK,
    isr  = __tn::TN_CONTEXT_ISR
};

enum
{
    no_time_slice  = TN_NO_TIME_SLICE,
    max_time_slice = TN_MAX_TIME_SLICE
};

void tick(void); // tn_tick_int_processing
rc tslice_set(const priority _priority, const int32_t _ticks);
uint32_t tick_get(void);
sys_state state_get(void);
context conext_get(void);
namespace sheduler
{
    void restore(void);
    void dis_save(void);
}

//-- task from tn_task.h --------------------------------------------------------------------------/

class task_base
{
protected:
    __tn::TN_Task task_;

    rc sleep(const uint32_t _tick);
    rc yield(void);
    void exit(void);
    void self_destructor(void);

public:
    enum class state: int8_t
    {
        none       = __tn::TN_TASK_STATE_NONE,
        runnable   = __tn::TN_TASK_STATE_RUNNABLE,
        wait       = __tn::TN_TASK_STATE_WAIT,
        suspend    = __tn::TN_TASK_STATE_SUSPEND,
        waitsusp   = __tn::TN_TASK_STATE_WAITSUSP,
        dormant    = __tn::TN_TASK_STATE_DORMANT,
        yield      = __tn::TN_TASK_STATE_YIELD,
        runtoyield = __tn::TN_TASK_STATE_RUNTOYIELD,
        err        = -1,
        
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


template <class T, uint32_t _stack_size, os::priority _priority = os::priority::normal>
class task : public task_base, private stack<_stack_size>
{
public:
    task(const char *const _name = nullptr)
    {
        if (tn_task_create_wname(
                &task_,
                member_to_func(&T::task_func),
                static_cast<int>(_priority),
                reinterpret_cast<__tn::TN_UWord *>(this->stack_ptr),
                this->stack_size / sizeof(uint32_t),
                this,
                _name
                ) != __tn::TN_RC_OK)
        {
            _TN_FATAL_ERROR("task not created");
        }
    }
};


template <class T, uint32_t _stack_size>
class kernel: public task<T, _stack_size, priority::idle>
{
public:
    kernel(uint32_t *const _sys_stack_ptr, const uint32_t _sys_stack_size):
        task<T, _stack_size, priority::idle>("idle")
    {
        __tn::tn_arch_int_dis();
        static_cast<T*>(this)->hw_init();
        __tn::tn_sys_start(
            _sys_stack_ptr,
            _sys_stack_size / 4,
            T::sw_init
            );
    }
};

//-- mutex from tn_mutex.h --------------------------------------------------------------------------/

class mutex
{
protected:
    __tn::TN_Mutex mutex_;

    enum class protocol
    {
        ceiling = __tn::TN_MUTEX_PROT_CEILING,
        inherit = __tn::TN_MUTEX_PROT_INHERIT
    };

public:
    rc lock(const uint32_t _timeout);
    rc lock_now();
    rc unlock();

    mutex(void);
    mutex(const priority _ceil_priority);

    ~mutex();
};

} // namespace os
