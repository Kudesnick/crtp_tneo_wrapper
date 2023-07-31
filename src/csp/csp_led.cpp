#include "RTE_Components.h"
#include CMSIS_device_header

#include "csp.h"
#include "csp_led_conf.h"

namespace csp //------------------------------------------------------------------------------------
{

namespace led //------------------------------------------------------------------------------------
{

res init(void)
{
    gpio::init(LED_PIN, 0, GPIO_MODE_OUTPUT_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);
    return res::ok;
}

void set(const bool _state)
{
    gpio::set(LED_PIN, _state ^ LED_INV);
}

void toggle(void)
{
    gpio::toggle(LED_PIN);
}

res deinit(void)
{
    gpio::init(LED_PIN, 0, MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);
    RCC->AHB1ENR &= ~gpio::rcc(LED_PIN);
    return res::ok;
}

}; // namespace led --------------------------------------------------------------------------------

}; // namespace csp --------------------------------------------------------------------------------