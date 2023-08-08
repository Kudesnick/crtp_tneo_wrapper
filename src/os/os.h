
#pragma once

#include <stdint.h>
#include <cstddef>

namespace __tn
{
#include "tn.h"
}

namespace os
{

//-- utils ----------------------------------------------------------------------------------------/

template <uint32_t sz>
struct stack
{
    static_assert(sz % sizeof(uint64_t) == 0, "mempool item must be a multiple of 8 bytes");
    static_assert(sz  / sizeof(uint32_t) >= TN_MIN_STACK_SIZE, "size of stack must be greater that TN_MIN_STACK_SIZE");
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
   ok          =  __tn::TN_RC_OK,
   timeout     =  __tn::TN_RC_TIMEOUT,
   overflow    =  __tn::TN_RC_OVERFLOW,
   wcontext    =  __tn::TN_RC_WCONTEXT,
   wstate      =  __tn::TN_RC_WSTATE,
   wparam      =  __tn::TN_RC_WPARAM,
   illegal_use =  __tn::TN_RC_ILLEGAL_USE,
   invalid_obj =  __tn::TN_RC_INVALID_OBJ,
   deleted     =  __tn::TN_RC_DELETED,
   forced      =  __tn::TN_RC_FORCED,
   internal    =  __tn::TN_RC_INTERNAL,
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
    dedlock = __tn::TN_STATE_FLAG__DEADLOCK,
};

enum class context: uint8_t
{
    none = __tn::TN_CONTEXT_NONE,
    task = __tn::TN_CONTEXT_TASK,
    isr  = __tn::TN_CONTEXT_ISR,
};

enum time_slise: uint16_t
{
    no_slice  = TN_NO_TIME_SLICE,
    max_slice = TN_MAX_TIME_SLICE,
};

enum wait: uint32_t
{
    nowait     = 0,
    infinitely = TN_WAIT_INFINITE,
};

enum repeat_timer: bool
{
    repeat = true,
    norepeat = false,
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
#if TN_DYNAMIC_TICK
    void cb_sleep_until(uint32_t _timestamp); // weak
    uint32_t cb_tick_get(void); // weak
#endif
}

#if TN_STACK_OVERFLOW_CHECK
class task_base;
void cb_stack_overflow(task_base *const _task); // weak
#endif

#if TN_MUTEX_DEADLOCK_DETECT
class mutex;
void cb_deadlock(const bool _active, mutex *const _mutex, task_base *const _task); // weak
#endif

//-- task from tn_task.h --------------------------------------------------------------------------/

class task_base
{
protected:
    __tn::TN_Task task_;

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

    rc static sleep(const uint32_t _tick);
    rc static yield(void);
    void static exit(void);
    void static self_destructor(void);

    rc suspend(void);
    rc resume(void);
    rc terminate(void);
    state state_get(void);
    rc change_priority(const priority _priority);
    rc wakeup(void);
    rc activate(void);
    rc release_wait(void);
    ~task_base();
};


template <class T, uint32_t _stack_size, os::priority _priority = os::priority::normal>
class task : public task_base, private stack<_stack_size>
{
public:
    task(const char *const _name = nullptr)
    {
        if (__tn::tn_task_create_wname(
                &task_,
                member_to_func(&T::task_func),
                static_cast<int>(_priority),
                reinterpret_cast<__tn::TN_UWord *>(this->stack_ptr),
                this->stack_size / sizeof(uint32_t),
                this,
                _name
                ) != __tn::TN_RC_OK)
        {
            _TN_FATAL_ERROR("task not created\n");
        }
    }
};

//-- kernel ---------------------------------------------------------------------------------------/

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

//-- mutex from tn_mutex.h ------------------------------------------------------------------------/

class mutex
{
protected:
    __tn::TN_Mutex mutex_;

    enum class protocol
    {
        ceiling = __tn::TN_MUTEX_PROT_CEILING,
        inherit = __tn::TN_MUTEX_PROT_INHERIT,
    };

public:
    rc acquire(const uint32_t _timeout = nowait);
    rc release(void);

    mutex(void);
    mutex(const priority _ceil_priority);

    ~mutex();
};

//-- semaphores from tn_sem.h ---------------------------------------------------------------------/

class semaphore
{
protected:
    __tn::TN_Sem sem_;

public:
    rc acquire(const uint32_t _timeout = nowait);
    rc release(void);

    semaphore(const uint32_t start_, const uint32_t max_);
    ~semaphore();
};

//-- memory pool from tn_fmem.h -------------------------------------------------------------------/

class fmem_base
{
protected:
    __tn::TN_FMem fmem_;
    int32_t leave(void);
    int32_t add(void);

    friend class queue_base;
    /// @todo заменить на
    /// friend rc queue_base::receive_release(fmem_base &_fmem, void *_p_data, const uint32_t _timeout)
public:

    rc acquire(void **_p_data, const uint32_t _timeout = nowait);
    rc acquire_memcpy(void **_p_data, void *_p_source, const uint32_t _timeout = nowait);
    rc release(void *_p_data);
    rc append(void *_p_data);

    int32_t free_cnt_get(void);
    int32_t used_cnt_get(void);

    fmem_base(void *_start_addr, uint32_t _block_size, uint32_t _blocks_cnt);
    ~fmem_base();
};


template <class T> class fmem_typed: public fmem_base
{
static_assert(sizeof(T) >= sizeof(void *), "mempool item must be 4 bytes or greater");
static_assert(sizeof(T) % sizeof(uint32_t) == 0, "size of mempool item must be a multiple of 4 bytes");

public:
    class item
    {
    private:
        fmem_typed<T> *owner_;
        T* ptr_;
    public:
        T*const &ptr    = ptr_;
        fmem_typed<T> *&owner = owner_;

        rc acquire(fmem_typed<T> &_fmem, const uint32_t _timeout = nowait)
        {
            if (owner_ != nullptr) return rc::illegal_use;
            
            rc res = _fmem.fmem_base::acquire(reinterpret_cast<void **>(&ptr_), _timeout);
            if (res == rc::ok) owner_ = &_fmem;

            return res;
        }

        rc release(void)
        {
            if (owner_ == nullptr) return rc::illegal_use;

            rc res = owner_->release(*ptr_);
            if (res == rc::ok)
            {
                owner_ = nullptr;
                ptr_ = nullptr;
            }

            return res;
        }

        rc move(fmem_typed<T> &_fmem)
        {
            if (owner_ == nullptr || ptr_ == nullptr) return rc::illegal_use;
            else if (owner_ == &_fmem) return rc::illegal_use;
            
            auto source_cnt = owner_->used_cnt_get();
            auto dest_cnt = _fmem.used_cnt_get();
            
            if (true
                && source_cnt > 0   
                && owner_->leave() == source_cnt - 1
                && _fmem.add() == dest_cnt + 1
            )
            {
                owner_ = &_fmem;
                return rc::illegal_use;
            }

            return rc::ok;
        }

        item(fmem_typed<T> &_fmem):
            owner_(nullptr), ptr_(nullptr)
        {
            acquire(_fmem, nowait);
        }
        
        item(void): owner_(nullptr), ptr_(nullptr) {}
    };

    rc acquire(T *&_p_data, const uint32_t _timeout = nowait)
    {
        return fmem_base::acquire(reinterpret_cast<void **>(&_p_data), _timeout);
    }

    rc release(T &_data)
    {
        return fmem_base::release(static_cast<void *>(&_data));
    }

    rc append(T &_data)
    {
        return fmem_base::append(static_cast<void *>(&_data));
    }

    rc acquire(item &_item, const uint32_t _timeout = nowait)
    {
        return _item.acquire(this, _timeout);
    }

    rc release(item &_item)
    {
        return (_item.owner == this) ? _item.release() : rc::illegal_use;
    }
    
    fmem_typed(T *const _start_addr, const uint32_t _blocks_cnt):
        fmem_base(static_cast<void *>(_start_addr), sizeof(T), _blocks_cnt){}
};


template <class T, uint32_t cnt> class fmem: public fmem_typed<T>
{
static_assert(cnt > 0, "count of mempool items must be greater than 0");

private:
    T pool_[cnt];
public:
    fmem(void): fmem_typed<T>(pool_, cnt){}
};

//-- event group from tn_eventgrp.h ---------------------------------------------------------------/

class eventgrp
{
private:
    __tn::TN_EventGrp eventgrp_;

    friend class queue_base;
    /// @todo заменить на:
    /// friend rc queue_base::evengrp_connect(eventgrp &_eventgrp, const uint32_t _pattern);

public:
    enum class wait_mode
    {
        w_or      = __tn::TN_EVENTGRP_WMODE_OR,
        w_and     = __tn::TN_EVENTGRP_WMODE_AND,
        w_or_clr  = __tn::TN_EVENTGRP_WMODE_OR  | __tn::TN_EVENTGRP_WMODE_AUTOCLR,
        w_and_clr = __tn::TN_EVENTGRP_WMODE_AND | __tn::TN_EVENTGRP_WMODE_AUTOCLR,
    };

    enum class op_mode
    {
        set    = __tn::TN_EVENTGRP_OP_SET,
        clr    = __tn::TN_EVENTGRP_OP_CLEAR,
        toggle = __tn::TN_EVENTGRP_OP_TOGGLE,
    };

    eventgrp(const uint32_t _pattern);
    rc wait(const uint32_t _pattern, const wait_mode _wait_mode, uint32_t *const _f_pattern, const uint32_t _timeout = nowait);
    rc modify(const op_mode _op_mode, const uint32_t _pattern);
    ~eventgrp();
};

//-- queue from tn_dqueue.h -----------------------------------------------------------------------/

class queue_base
{
private:
    __tn::TN_DQueue queue_;
protected:
    queue_base(void **_p_fifo, const uint32_t);
    rc send(void *const _p_data, const uint32_t _timeout = nowait);
    rc receive(void **_pp_data, const uint32_t _timeout = nowait);
    rc send_acquire(fmem_base &_fmem, void *_p_data, const uint32_t _timeout = nowait);
    rc receive_release(fmem_base &_fmem, void *_p_data, const uint32_t _timeout = nowait);
public:
    int32_t free_cnt_get(void);
    int32_t used_cnt_get(void);
    rc evengrp_connect(eventgrp &_eventgrp, const uint32_t _pattern);
    rc evengrp_disconnect(void);
    ~queue_base();
};


template <class T> class queue_typed: public queue_base
{
static_assert(sizeof(T) <= sizeof(void *), "size of queue's item type must be less or equal than 4 bytes");

public:
    queue_typed(void *_p_fifo = nullptr, const uint32_t _items = 0): queue_base(&_p_fifo, _items)
    {}

    rc send(T &_data, const uint32_t _timeout = nowait)
    {
        return queue_base::send(reinterpret_cast<void *>(_data), _timeout);
    }
    
    rc receive(T &_data, const uint32_t _timeout = nowait)
    {
        return queue_base::receive(reinterpret_cast<void **>(&_data), _timeout);
    }
};


template <class T, uint32_t cnt> class queue: public queue_typed<T>
{
private:
    void *fifo_[cnt];
public:
    queue(): queue_typed<T>(fifo_, cnt){}
};


template <class T> class fmem_queue_typed: public queue_base
{
public:
    fmem_typed<T> &fmem;

    fmem_queue_typed(fmem_typed<T> &_fmem, T *_p_fifo = nullptr, const uint32_t _items = 0):
        fmem(_fmem),
        queue_base(&_p_fifo, _items)
    {}

    rc send_acquire(T &_data, const uint32_t _timeout = nowait)
    {
        return queue_base::send_acquire(fmem, &_data, _timeout);
    }
    
    rc receive_release(T &_data, const uint32_t _timeout = nowait)
    {
        return queue_base::receive_release(fmem, &_data, _timeout);
    }
};


template <class T, uint32_t queue_cnt, uint32_t fmem_cnt> class fmem_queue: public fmem_queue_typed<T>
{
private:
    void *fifo_[queue_cnt];
    fmem<T, fmem_cnt> fmem_;
public:
    fmem_queue(): fmem_queue_typed<T>(fmem_, fifo_, queue_cnt){}
};

//-- timers from tn_tmer.h ------------------------------------------------------------------------/

class timer_base
{
private:
    __tn::TN_Timer timer_;
    uint32_t timeout_;
    repeat_timer repeat_;

    static void handler_(__tn::TN_Timer *_timer, void *_arg);

public:
    timer_base(void(*_func)(void*), const uint32_t _timeout = os::nowait, const repeat_timer _repeat = os::norepeat);
    rc start(void);
    rc start(const uint32_t _timeout);
    rc start(const uint32_t _timeout, const repeat_timer _repeat);
 	rc cancel(void);
 	bool is_active(void);
 	uint32_t time_left(void);
    ~timer_base();
};


template <class T, const uint32_t _timeout = os::nowait, const repeat_timer _repeat = os::norepeat> class timer : public timer_base
{
public:
    timer():
        timer_base(member_to_func(&T::timer_func), _timeout, _repeat){}
};


} // namespace os
