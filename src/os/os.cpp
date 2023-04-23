
#include <stdint.h>
#include "os.h"
#include "tn.h"

namespace os
{

//-- task_base ----------------------------------------------------------------/

rc task_base::sleep(const uint32_t _tick)
{
    return static_cast<rc>(tn_task_sleep(_tick));
}

rc task_base::yield(void)
{
    return static_cast<rc>(tn_task_yield());
}

void task_base::exit(void)
{
    tn_task_exit(TN_TASK_EXIT_OPT_NO_DELETE);
}

void task_base::self_destructor(void)
{
    tn_task_exit(TN_TASK_EXIT_OPT_DELETE);
}

rc task_base::suspend(void)
{
    return static_cast<rc>(tn_task_suspend(&task_));
}

rc task_base::resume(void)
{
    return static_cast<rc>(tn_task_resume(&task_));
}

rc task_base::terminate(void)
{
    return static_cast<rc>(tn_task_terminate(&task_));
}

task_base::state task_base::state_get(void)
{
    auto r_state = static_cast<enum TN_TaskState>(state::err);

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
        tn_is_task_context() ? tn_task_wakeup(&task_) : tn_task_iwakeup(&task_)
        );
}

rc task_base::activate(void)
{
    return static_cast<rc>(
        tn_is_task_context() ? tn_task_activate(&task_) : tn_task_iactivate(&task_)
        );
}

rc task_base::release_wait(void)
{
    return static_cast<rc>(
        tn_is_task_context() ? tn_task_release_wait(&task_) : tn_task_irelease_wait(&task_)
        );
}

bool task_base::is_task_context(void)
{
    return tn_is_task_context();
}

task_base::~task_base()
{
    tn_task_delete(&task_);
}

} // namespace os
