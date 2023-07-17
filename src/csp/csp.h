#pragma once

#include <stdint.h>
#include "misc.h"

namespace csp
{

extern uint32_t *const &stack_ptr;
extern const uint32_t stack_size;

namespace tick
{
    res init(const uint32_t _ms);
    void cb_tick_handl(void); // weak
    uint32_t tick_get(void);
};

void halt(void);

void interrupt_global(const bool _enable);

}; // namespace csp
