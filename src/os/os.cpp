#include "RTE_Components.h"
#include CMSIS_device_header

#if !defined(__CC_ARM) && defined(__ARMCC_VERSION) && !defined(__OPTIMIZE__)
#warning This project uses syntax solutions that require an optimization level of at least 1. \
Otherwise, you will see an increase in ROM and RAM consumption up to two sizes. \
As well as unjustified performance degradation.
#endif

#include <stdint.h>
#include <cstring>
#include "os.h"
#include "misc.h"

namespace os
{

//-- common data ----------------------------------------------------------------------------------/

void tick(void)
{
    __tn::tn_tick_int_processing();
}

rc tslice_set(const priority _priority, const int32_t _ticks)
{
    return static_cast<rc>(__tn::tn_sys_tslice_set(static_cast<int>(_priority), _ticks));
}

uint32_t tick_get(void)
{
    return __tn::tn_sys_time_get();
}

sys_state state_get(void)
{
    return static_cast<sys_state>(__tn::tn_sys_state_flags_get());
}

context conext_get(void)
{
    return static_cast<context>(__tn::tn_sys_context_get());
}

namespace sheduler
{
    static uint32_t shed_state;
    
    void restore(void)
    {
        __tn::tn_sched_restore(shed_state);
    }
    void dis_save(void)
    {
        shed_state = __tn::tn_sched_dis_save();
    }
#if TN_DYNAMIC_TICK
    __WEAK void cb_sleep_until(uint32_t _timestamp)
    {
        (void)_timestamp;
        PRINTFAULT("function 'sleep_until' must be overloaded\n");
    }
    __WEAK uint32_t cb_tick_get(void)
    {
        PRINTFAULT("function 'cb_tick_get' must be overloaded\n");
        return 0;
    }
#endif
}


#if TN_STACK_OVERFLOW_CHECK
__WEAK __INLINE void cb_stack_overflow(task_base *const _task)
{
    (void)_task;
}
#endif

#if TN_MUTEX_DEADLOCK_DETECT
__WEAK __INLINE void cb_deadlock(const bool _active, mutex *const _mutex, task_base *const _task)
{
    (void)_active;
    (void)_mutex;
    (void)_task;
}
#endif

//-- task_base ------------------------------------------------------------------------------------/

rc task_base::sleep(const uint32_t _tick)
{
    return static_cast<rc>(__tn::tn_task_sleep(_tick));
}

rc task_base::yield(void)
{
    return static_cast<rc>(__tn::tn_task_yield());
}

void task_base::exit(void)
{
    tn_task_exit(__tn::TN_TASK_EXIT_OPT_NO_DELETE);
}

void task_base::self_destructor(void)
{
    tn_task_exit(__tn::TN_TASK_EXIT_OPT_DELETE);
}

rc task_base::suspend(void)
{
    return static_cast<rc>(__tn::tn_task_suspend(&task_));
}

rc task_base::resume(void)
{
    return static_cast<rc>(__tn::tn_task_resume(&task_));
}

rc task_base::terminate(void)
{
    return static_cast<rc>(__tn::tn_task_terminate(&task_));
}

task_base::state task_base::state_get(void)
{
    auto r_state = state::err;

    tn_task_state_get(&task_, reinterpret_cast<enum __tn::TN_TaskState *>(&r_state));
    return r_state;
}

rc task_base::change_priority(const priority _priority)
{
    return static_cast<rc>(__tn::tn_task_change_priority(&task_, static_cast<int>(_priority)));
}

rc task_base::wakeup(void)
{
    return static_cast<rc>(__tn::tn_is_task_context() ? __tn::tn_task_wakeup(&task_) : __tn::tn_task_iwakeup(&task_));
}

rc task_base::activate(void)
{
    return static_cast<rc>(__tn::tn_is_task_context() ? __tn::tn_task_activate(&task_) : __tn::tn_task_iactivate(&task_));
}

rc task_base::release_wait(void)
{
    return static_cast<rc>(__tn::tn_is_task_context() ? __tn::tn_task_release_wait(&task_) : __tn::tn_task_irelease_wait(&task_));
}

task_base::~task_base()
{
    if (__tn::tn_task_delete(&task_) != __tn::TN_RC_OK)
    {
        PRINTFAULT("rask destructor error\n");
    }
}

//-- mutex ----------------------------------------------------------------------------------------/

rc mutex::acquire(const uint32_t _timeout)
{
    return static_cast<rc>(tn_mutex_lock(&mutex_, _timeout));
}

rc mutex::release(void)
{
    return static_cast<rc>(tn_mutex_unlock(&mutex_));
}

mutex::mutex(void)
{
    if (__tn::tn_mutex_create(&mutex_, __tn::TN_MUTEX_PROT_INHERIT, 0) != __tn::TN_RC_OK)
    {
        PRINTFAULT("mutex not created\n");
    }
}

mutex::mutex(const priority _ceil_priority)
{
    if (__tn::tn_mutex_create(&mutex_, __tn::TN_MUTEX_PROT_CEILING, static_cast<int>(_ceil_priority)) != __tn::TN_RC_OK)
    {
        PRINTFAULT("mutex not created\n");
    }
}

mutex::~mutex()
{
    if (__tn::tn_mutex_delete(&mutex_) != __tn::TN_RC_OK)
    {
        PRINTFAULT("mutex destructor error\n");
    }
}

//-- semaphore ------------------------------------------------------------------------------------/

rc semaphore::acquire(const uint32_t _timeout)
{
    if (__tn::tn_is_task_context())
    {
        return static_cast<rc>(__tn::tn_sem_wait(&sem_, _timeout));
    }
    else if (_timeout == os::nowait)
    {
        return static_cast<rc>(__tn::tn_sem_iwait_polling(&sem_));
    }
    else
    {
        return rc::wparam;
    }
}

rc semaphore::release(void) // increment
{
    return static_cast<rc>(__tn::tn_is_task_context() ? __tn::tn_sem_signal(&sem_) : __tn::tn_sem_isignal(&sem_));
}

semaphore::semaphore(const uint32_t start_, const uint32_t max_)
{
    if (__tn::tn_sem_create(&sem_, start_, max_) != __tn::TN_RC_OK)
    {
        PRINTFAULT("semaphore not created\n");
    }
}

semaphore::~semaphore()
{
    if (__tn::tn_sem_delete(&sem_)  != __tn::TN_RC_OK)
    {
        PRINTFAULT("semaphore destructor error\n");
    }
}

//-- fmem -----------------------------------------------------------------------------------------/

int32_t fmem_base::leave(void)
{
    return (fmem_.blocks_cnt) ? static_cast<int32_t>(--fmem_.blocks_cnt) : -1;
}

int32_t fmem_base::add(void)
{
    return static_cast<int32_t>(++fmem_.blocks_cnt);
}

rc fmem_base::acquire(void **_p_data, const uint32_t _timeout)
{
    if (_timeout == os::nowait)
    {
        return static_cast<rc>(__tn::tn_is_task_context() ? __tn::tn_fmem_get_polling(&fmem_, _p_data) : __tn::tn_fmem_iget_polling(&fmem_, _p_data));
    }
    else if (__tn::tn_is_task_context())
    {
        return static_cast<rc>(__tn::tn_fmem_get(&fmem_, _p_data, _timeout));
    }
    else
    {
        return rc::wparam;
    }
}

rc fmem_base::acquire_memcpy(void **_p_data, void *_p_source, const uint32_t _timeout)
{
    rc ret = acquire(_p_data, _timeout);
    
    if (ret == rc::ok)
    {
        std::memcpy(*_p_data, _p_source, fmem_.block_size);
    }
    
    return ret;
}

rc fmem_base::release(void *_p_data)
{
    return static_cast<rc>(__tn::tn_is_task_context() ? __tn::tn_fmem_release(&fmem_, _p_data) : __tn::tn_fmem_irelease(&fmem_, _p_data));
}

rc fmem_base::append(void *_p_data)
{
    return static_cast<rc>(__tn::tn_is_task_context() ? __tn::tn_fmem_append(&fmem_, _p_data) : __tn::tn_fmem_iappend(&fmem_, _p_data));
}

rc fmem_base::reset(void)
{
    return static_cast<rc>(__tn::tn_fmem_reset(&fmem_));
}

int32_t fmem_base::free_cnt_get(void)
{
    return __tn::tn_fmem_free_blocks_cnt_get(&fmem_);
}

int32_t fmem_base::used_cnt_get(void)
{
    return __tn::tn_fmem_used_blocks_cnt_get(&fmem_);
}

fmem_base::fmem_base(void *_start_addr, uint32_t _block_size, uint32_t _blocks_cnt)
{
    if (__tn::tn_fmem_create(&fmem_, _start_addr, _block_size, _blocks_cnt) != __tn::TN_RC_OK)
    {
         PRINTFAULT("fmem pool not created\n");
    }
}

fmem_base::~fmem_base()
{
    if (__tn::tn_fmem_delete(&fmem_)  != __tn::TN_RC_OK)
    {
        PRINTFAULT("fmem pool destructor error\n");
    }
}

//-- event group from tn_eventgrp.h ---------------------------------------------------------------/

eventgrp::eventgrp(void): eventgrp(0) {}

eventgrp::eventgrp(const uint32_t _pattern)
{
    if (__tn::tn_eventgrp_create(&eventgrp_, _pattern) != __tn::TN_RC_OK)
    {
         PRINTFAULT("event group not created\n");
    }
}

rc eventgrp::modify(const op_mode _op_mode, const uint32_t _pattern)
{
    return static_cast<rc>(__tn::tn_is_task_context() ?
        __tn::tn_eventgrp_modify(&eventgrp_,  static_cast<__tn::TN_EGrpOp>(_op_mode), _pattern) :
        __tn::tn_eventgrp_imodify(&eventgrp_, static_cast<__tn::TN_EGrpOp>(_op_mode), _pattern));
}

rc eventgrp::wait(const uint32_t _pattern, const wait_mode _wait_mode, const uint32_t _timeout, uint32_t *const _f_pattern)
{
    if (_timeout == os::nowait)
    {
        return static_cast<rc>(__tn::tn_is_task_context() ?
            __tn::tn_eventgrp_wait_polling(&eventgrp_, _pattern, static_cast<__tn::TN_EGrpWaitMode>(_wait_mode), _f_pattern) :
            __tn::tn_eventgrp_iwait_polling(&eventgrp_, _pattern, static_cast<__tn::TN_EGrpWaitMode>(_wait_mode), _f_pattern));
    }
    else if (__tn::tn_is_task_context())
    {
        return static_cast<rc>(__tn::tn_eventgrp_wait(&eventgrp_, _pattern, static_cast<__tn::TN_EGrpWaitMode>(_wait_mode), _f_pattern, _timeout));
    }
    else
    {
        return rc::wparam;
    }
}

rc eventgrp::set(const uint32_t _pattern) {return modify(op_mode::set, _pattern);}
rc eventgrp::clr(const uint32_t _pattern) {return modify(op_mode::clr, _pattern);}
rc eventgrp::toggle(const uint32_t _pattern) {return modify(op_mode::toggle, _pattern);}

eventgrp::~eventgrp()
{
    if (__tn::tn_eventgrp_delete(&eventgrp_)  != __tn::TN_RC_OK)
    {
        PRINTFAULT("event group destructor error\n");
    }
}

//-- queue from tn_dqueue.h -----------------------------------------------------------------------/

queue_base::queue_base(void **_pp_fifo, const uint32_t _items)
{
    if (__tn::tn_queue_create(&queue_, _pp_fifo, static_cast<int32_t>(_items)) != __tn::TN_RC_OK)
    {
         PRINTFAULT("queue not created\n");
    }
}

rc queue_base::send(void *const _p_data, const uint32_t _timeout)
{
    if (__tn::tn_is_task_context())
    {
        return static_cast<rc>(__tn::tn_queue_send(&queue_, _p_data, _timeout));
    }
    else if (_timeout == os::nowait)
    {
        return static_cast<rc>(__tn::tn_queue_isend_polling(&queue_, _p_data));
    }
    else
    {
        return rc::wparam;
    }
}

rc queue_base::receive(void **_pp_data, const uint32_t _timeout)
{
    if (__tn::tn_is_task_context())
    {
        return static_cast<rc>(__tn::tn_queue_receive(&queue_, _pp_data, _timeout));
    }
    else if (_timeout == os::nowait)
    {
        return static_cast<rc>(__tn::tn_queue_ireceive_polling(&queue_, _pp_data));
    }
    else
    {
        return rc::wparam;
    }
}

rc queue_base::send_acquire(fmem_base &_fmem, void *_p_data, const uint32_t _timeout)
{
    uint32_t timestamp = tick_get();
    void *ptr;
    rc ret = _fmem.acquire_memcpy(&ptr, _p_data, _timeout);

    if (ret == rc::ok)
    {
        uint32_t timeout = (_timeout == nowait || _timeout == infinitely) ? _timeout :
            _timeout - (tick_get() - timestamp);
        if (timeout > _timeout) timeout = 0;
        
        ret = queue_base::send(ptr, timeout);
        
        if (ret != rc::ok) _fmem.release(ptr);
    }

    return ret;
}

rc queue_base::receive_release(fmem_base &_fmem, void *_p_data, const uint32_t _timeout)
{
    void *ptr;
    rc ret = queue_base::receive(&ptr, _timeout);
    
    if (ret == rc::ok)
    {
        memcpy(_p_data, ptr, _fmem.fmem_.block_size);
        ret = _fmem.release(ptr);
    }
    
    return ret;
}

int32_t queue_base::free_cnt_get(void)
{
    return __tn::tn_queue_free_items_cnt_get(&queue_);
}

int32_t queue_base::used_cnt_get(void)
{
    return __tn::tn_queue_used_items_cnt_get(&queue_);
}

rc queue_base::evengrp_connect(eventgrp &_eventgrp, const uint32_t _pattern)
{
    return static_cast<rc>(
        __tn::tn_queue_eventgrp_connect(&queue_, &_eventgrp.eventgrp_, _pattern));
}

rc queue_base::evengrp_disconnect(void)
{
    return static_cast<rc>(__tn::tn_queue_eventgrp_disconnect(&queue_));
}

rc queue_base::reset(void)
{
    return static_cast<rc>(__tn::tn_queue_reset(&queue_));
}

queue_base::~queue_base()
{
    if (__tn::tn_queue_delete(&queue_) != __tn::TN_RC_OK)
    {
         PRINTFAULT("queue destructor error\n");
    }
}

//-- timers from tn_timer.h ------------------------------------------------------------------------/

void timer_base::handler_(__tn::TN_Timer *_timer, void *_func)
{
    timer_base *const &timer_obj = reinterpret_cast<timer_base *>(_timer);
    
    if (_func)
    {
        reinterpret_cast<void(*)(void*)>(_func)(timer_obj);
    }
    
    if (timer_obj->repeat_)
    {
        timer_obj->start(timer_obj->timeout_);
    }
}

timer_base::timer_base(void(*_func)(void*), const uint32_t _timeout, const repeat_timer _repeat):
    timeout_(_timeout), repeat_(_repeat)
{
    if (__tn::tn_timer_create(
            &timer_,
            handler_,
            reinterpret_cast<void *>(_func)
            ) != __tn::TN_RC_OK)
    {
        PRINTFAULT("timer not created\n");
    }
}

rc timer_base::start(void)
{
    return static_cast<rc>(__tn::tn_timer_start(&timer_, timeout_));
}

rc timer_base::start(const uint32_t _timeout)
{
    timeout_ = _timeout;
    return static_cast<rc>(__tn::tn_timer_start(&timer_, _timeout));
}

rc timer_base::start(const uint32_t _timeout, const repeat_timer _repeat)
{
    timeout_ = _timeout;
    repeat_ = _repeat;
    return static_cast<rc>(__tn::tn_timer_start(&timer_, _timeout));
}

rc timer_base::cancel()
{
    return static_cast<rc>(__tn::tn_timer_cancel(&timer_));
}

bool timer_base::is_active(void)
{
    bool result = false;
    __tn::tn_timer_is_active(&timer_, &result);
    return result;
}

uint32_t timer_base::time_left(void)
{
    __tn::TN_TickCnt result = os::infinitely;
    __tn::tn_timer_time_left(&timer_, &result);
    return result;
}

timer_base::~timer_base()
{
    if (__tn::tn_timer_delete(&timer_) != __tn::TN_RC_OK)
    {
         PRINTFAULT("timer destructor error\n");
    }
}

} // namespace os

//-- override weak functions ----------------------------------------------------------------------/

#if TN_STACK_OVERFLOW_CHECK
void __tn::tn_cb_stack_overflow(__tn::TN_Task *_task)
{
    /** @todo Опасное преобразование! Будет работать только если поле __tn::TN_Task task_ будет
     * первым в классе task_base и класс task_base не будет содержать виртуальных методов
     */
    os::cb_stack_overflow(reinterpret_cast<os::task_base *>(_task));
}
#endif

#if TN_MUTEX_DEADLOCK_DETECT
void __tn::tn_cb_deadlock(TN_BOOL _active, struct __tn::TN_Mutex *_mutex, struct __tn::TN_Task *_task)
{
    /** @todo Опасное преобразование! Будет работать только если поле __tn::TN_Task task_ будет
     * первым в классе task_base и класс task_base не будет содержать виртуальных методов
     */
    /** @todo Опасное преобразование! Будет работать только если поле __tn::TN_Mutex mutex_
     * будет первым в классе mutex и класс mutex не будет содержать виртуальных методов
     */
    os::cb_deadlock(_active, reinterpret_cast<os::mutex *>(_mutex), reinterpret_cast<os::task_base *>(_task));
}
#endif

#if TN_DYNAMIC_TICK
void __tn::tn_cb_tick_schedule(__tn::TN_TickCnt timeout)
{
    os::sheduler::cb_sleep_until(timeout);
}

uint32_t __tn::tn_cb_tick_cnt_get(void)
{
    return os::sheduler::cb_tick_get();
}
#endif
