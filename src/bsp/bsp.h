#pragma once

#include <stdint.h>

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

class nor_flash
{
public:
    nor_flash(void);
    ~nor_flash(void);
};

}; // namespace bsp
