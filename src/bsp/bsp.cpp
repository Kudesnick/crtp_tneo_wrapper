#include "RTE_Components.h"
#include CMSIS_device_header

#include "bsp.h"
#include "csp_led.h"

namespace bsp //------------------------------------------------------------------------------------
{

led::led(void)
{
    csp::led::init();
}

led::~led(void)
{
    csp::led::deinit();
}

void led::on(void)
{
    csp::led::set(true);
}

void led::off(void)
{
    csp::led::set(false);
}

void led::toggle(void)
{
    csp::led::toggle();
}

}; // namespace bsp --------------------------------------------------------------------------------
