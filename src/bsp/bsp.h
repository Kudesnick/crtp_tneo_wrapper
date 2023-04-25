#pragma once

#include <stdint.h>

namespace csp
{

enum class res
{
    ok  =  0,
    err = -1
};

extern uint32_t *const &stack_ptr;
extern const uint32_t stack_size;

namespace tick
{
    res init(const uint32_t _ms);
    void cb_tick_handl(void); // weak
};

void halt(void);

void interrupt_global(const bool _enable);

}; // namespace csp

namespace bsp
{

class led
{
public:
    led(void);
    ~led(void);
    void on(void);
    void off(void);
    void toggle(void);
};

}; // namespace bsp
