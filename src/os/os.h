
#pragma once

#include <stdint.h>
#include <cstddef>

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
#endif
}

class task_base;
class mutex;

void cb_stack_overflow(task_base *const _task); // weak
void cb_deadlock(const bool _active, mutex *const _mutex, task_base *const _task); // weak
#if TN_DYNAMIC_TICK
uint32_t cb_tick_get(void) // weak
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

//-- kernel ---------------------------------------------------------------------------------------/

template <class T, uint32_t _stack_size>
class kernel: public task<T, _stack_size, priority::idle>
{
private:
    static void cb_stack_overflow(__tn::TN_Task *_task)
    {
        /** @todo Опасное преобразование! Будет работать только если поле __tn::TN_Task task_ будет
         * первым в классе task_base и класс task_base не будет содержать виртуальных методов
         */
        os::cb_stack_overflow(reinterpret_cast<task_base *>(_task));
    }

    static void cb_deadlock(TN_BOOL _active, struct __tn::TN_Mutex *_mutex, struct __tn::TN_Task *_task)
    {
        /** @todo Опасное преобразование! Будет работать только если поле __tn::TN_Task task_ будет
         * первым в классе task_base и класс task_base не будет содержать виртуальных методов
         */
        /** @todo Опасное преобразование! Будет работать только если поле __tn::TN_Mutex mutex_
         * будет первым в классе mutex и класс mutex не будет содержать виртуальных методов
         */
        os::cb_deadlock(_active, reinterpret_cast<mutex *>(_mutex), reinterpret_cast<task_base *>(_task));
    }

public:
    kernel(uint32_t *const _sys_stack_ptr, const uint32_t _sys_stack_size):
        task<T, _stack_size, priority::idle>("idle")
    {
        __tn::tn_arch_int_dis();
        static_cast<T*>(this)->hw_init();
        __tn::tn_callback_stack_overflow_set(cb_stack_overflow);
        __tn::tn_callback_deadlock_set(cb_deadlock);
#if TN_DYNAMIC_TICK
        __tn::tn_callback_dyn_tick_set(sheduler::cb_sleep_until, cb_tick_get);
#endif
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
    rc acquire(const uint32_t _timeout);
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
    rc acquire(const uint32_t _timeout);
    rc release(void);

    semaphore(const uint32_t start_, const uint32_t max_);
    ~semaphore();
};

//-- memory pool from tn_fmem.h -------------------------------------------------------------------/

class fmem_base
{
protected:
    __tn::TN_FMem fmem_;

public:
    rc acquire(void **_p_data, const uint32_t _timeout);
    rc release(void *_p_data);
    rc append(void *_p_data);

    int32_t free_cnt_get(void);
    int32_t used_cnt_get(void);

    fmem_base(void *_start_addr, uint32_t _block_size, uint32_t _blocks_cnt);
    ~fmem_base();
};


template <class T> class fmem: public fmem_base
{
    static_assert(sizeof(T) >= sizeof(void *), "mempool item must be 4 bytes or greater");
static_assert(sizeof(T) % sizeof(uint32_t) == 0, "size of mempool item must be a multiple of 4 bytes");
public:
    class item
    {
    private:
        fmem<T> *owner_;
        T* ptr_;
    public:
        T*const &ptr    = ptr_;
        fmem<T> *&owner = owner_;

        rc acquire(fmem<T> &_fmem, const uint32_t _timeout)
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

        rc move(fmem<T> &_fmem)
        {
            if (owner_ == nullptr) return rc::illegal_use;
            else if (owner_ == &_fmem) return release();
            
            rc res = owner_->append(*ptr_);
            if (res == rc::ok)
            {
                owner_ = nullptr;
                ptr_ = nullptr;
            }

            return res;
        }

        item(fmem<T> &_fmem):
            owner_(nullptr), ptr_(nullptr)
        {
            acquire(_fmem, nowait);
        }
        
        item(void): owner_(nullptr), ptr_(nullptr) {}
    };

    rc acquire(T *&_p_data, const uint32_t _timeout)
    {
        return fmem_base::acquire(reinterpret_cast<void **>(&_p_data), _timeout);
    }

    rc release(T &_p_data)
    {
        return fmem_base::release(static_cast<void *>(&_p_data));
    }

    rc append(T &_p_data)
    {
        return fmem_base::append(static_cast<void *>(&_p_data));
    }

    rc acquire(item &_item, const uint32_t _timeout)
    {
        return _item.acquire(this, _timeout);
    }

    rc release(item &_item)
    {
        return (_item.owner == this) ? _item.release() : rc::illegal_use;
    }
    
    fmem(T *const _start_addr, const uint32_t _blocks_cnt):
        fmem_base(static_cast<void *>(_start_addr), sizeof(T), _blocks_cnt){}
};


template <class T, uint32_t cnt> class fmempool: public fmem<T>
{
static_assert(cnt > 0, "count of mempool items must be greater that 1");
private:
    T pool_[cnt];
public:
    fmempool(void): fmem<T>(pool_, cnt){}
};

} // namespace os
