
#include <stdint.h>
#include "os.h"

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
}


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
    auto r_state = static_cast<enum __tn::TN_TaskState>(state::err);

    tn_task_state_get(&task_, &r_state);
    return static_cast<state>(r_state);
}

rc task_base::change_priority(const priority _priority)
{
    return static_cast<rc>(tn_task_change_priority(&task_, static_cast<int>(_priority)));
}

rc task_base::wakeup(void)
{
    return static_cast<rc>(
        __tn::tn_is_task_context() ? tn_task_wakeup(&task_) : tn_task_iwakeup(&task_)
        );
}

rc task_base::activate(void)
{
    return static_cast<rc>(
        __tn::tn_is_task_context() ? tn_task_activate(&task_) : tn_task_iactivate(&task_)
        );
}

rc task_base::release_wait(void)
{
    return static_cast<rc>(
        __tn::tn_is_task_context() ? tn_task_release_wait(&task_) : tn_task_irelease_wait(&task_)
        );
}

bool task_base::is_task_context(void)
{
    return __tn::tn_is_task_context();
}

task_base::~task_base()
{
    tn_task_delete(&task_);
}

//-- mutex ----------------------------------------------------------------------------------------/

rc mutex::lock(const uint32_t _timeout)
{
    return static_cast<rc>(tn_mutex_lock(&mutex_, _timeout));
}

rc mutex::lock_now()
{
    return static_cast<rc>(tn_mutex_lock_polling(&mutex_));
}

rc mutex::unlock()
{
    return static_cast<rc>(tn_mutex_unlock(&mutex_));
}

mutex::mutex(void)
{
    if (__tn::tn_mutex_create(&mutex_, __tn::TN_MUTEX_PROT_INHERIT, 0) != __tn::TN_RC_OK)
    {
        _TN_FATAL_ERROR("mutex not created");
    }
}

mutex::mutex(const priority _ceil_priority)
{
    if (__tn::tn_mutex_create(
            &mutex_,
            __tn::TN_MUTEX_PROT_CEILING,
            static_cast<int>(_ceil_priority)
            ) != __tn::TN_RC_OK)
    {
        _TN_FATAL_ERROR("mutex not created");
    }
}

mutex::~mutex()
{
    __tn::tn_mutex_delete(&mutex_);
}

} // namespace os
