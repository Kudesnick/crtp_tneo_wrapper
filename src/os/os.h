
#include <stdint.h>
#include "tn.h"

namespace os
{
    
enum class priority: uint8_t
{
    idle     = 0,
    low      = 1,
    normal   = 2,
    high     = 3,
    relatime = 4,
};
    
template <typename T> class kernel
{
public:
    static inline void tick(void)
    {
        tn_tick_int_processing();
    };

    kernel(void)
    {
        tn_arch_int_dis();
        T *const &base = static_cast<T*>(this);
        base->init_hw();
        tn_sys_start(
            base->idle_stack,
            sizeof(base->idle_stack),
            base->int_stack,
            sizeof(base->int_stack),
            base->idle_task,
            base->init_sw
            );
    };
};

} // namespace os
