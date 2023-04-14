#pragma once

#include <stdint.h>

namespace csp
{
extern uint32_t *const &stack_ptr;
extern const uint32_t stack_size;

class tick
{
private:
    tick(const uint32_t _ms, void(*const _handle)(void));
    tick();
    tick( const tick& );  
    tick& operator=( tick& );
public:
    static tick& init(const uint32_t _ms, void(*const _handle)(void));
    static void(*handle)(void);
};

void halt(void);

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
