#include "RTE_Components.h"
#include CMSIS_device_header

#if !defined(__CC_ARM) && defined(__ARMCC_VERSION) && !defined(__OPTIMIZE__)
    #error This project uses syntax solutions that require an optimization level of at least 1.
    #error Otherwise, you will see an increase in ROM and RAM consumption up to two sizes.
    #error As well as unjustified performance degradation.
#endif

#include <stdint.h>
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
        PRINTFAULT("function 'sleep_until' must be overloaded\n");
    }
#endif
}

__WEAK void cb_stack_overflow(task_base *const _task)
{
    (void)_task;
}

__WEAK void cb_deadlock(const bool _active, mutex *const _mutex, task_base *const _task)
{
    (void)_active;
    (void)_mutex;
    (void)_task;
}

#if TN_DYNAMIC_TICK
    __WEAK uint32_t cb_tick_get(void)
    {
        PRINTFAULT("function 'cb_tick_get' must be overloaded\n");
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
    else if (_timeout == 0)
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

rc fmem_base::acquire(void **_p_data, const uint32_t _timeout)
{
    if (_timeout == 0)
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

rc fmem_base::release(void *_p_data)
{
    return static_cast<rc>(__tn::tn_is_task_context() ? __tn::tn_fmem_release(&fmem_, _p_data) : __tn::tn_fmem_irelease(&fmem_, _p_data));
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

} // namespace os

//-- override weak functions ----------------------------------------------------------------------/
