#pragma once

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
