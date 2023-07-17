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
    GPIO_AF_INIT(LED_PIN, 0, GPIO_MODE_OUTPUT_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);
    return res::ok;
}

void set(const bool _state)
{
    GPIO_SET(LED_PIN, _state ^ LED_INV);
}

void toggle(void)
{
    GPIO_TOGGLE(LED_PIN);
}

res deinit(void)
{
    GPIO_AF_INIT(LED_PIN, 0, MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);
    RCC->AHB1ENR &= ~GPIO_PORT_RCC(LED_PIN);
    return res::ok;
}

}; // namespace led --------------------------------------------------------------------------------

}; // namespace csp --------------------------------------------------------------------------------