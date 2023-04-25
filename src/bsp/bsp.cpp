#include "RTE_Components.h"
#include CMSIS_device_header

#include "bsp.h"
#include "misc.h"

extern "C" const uint32_t Image$$RW_STACK$$Base;
extern "C" const uint32_t Image$$RW_STACK$$Length;

static void dummy(void) {};

namespace csp
{
uint32_t *const &stack_ptr = const_cast<uint32_t *>(&Image$$RW_STACK$$Base);
const uint32_t stack_size = Image$$RW_STACK$$Length - reinterpret_cast<uint32_t>(&Image$$RW_STACK$$Base);

res tick::init(const uint32_t _ms)
{
    auto res = res::ok;
    
    if (SysTick_Config(HSE_VALUE * _ms / 1000) != 0)
    {
        res = res::err;
        PRINTFAULT("value %d not corrected for systick interval\n", _ms);
    }
    return res;
}

__WEAK void tick::cb_tick_handl(void) {};


void halt(void)
{
#ifdef DEBUG
    __NOP();
#else
    __WFE();
#endif
}

void interrupt_global(const bool _enable)
{
    if (_enable)
    {
        __enable_irq();
    }
    else
    {
        __disable_irq();
    }
}

}; // namespace csp

namespace bsp
{

led::led(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct =
    {
        .Pin   = GPIO_PIN_13,
        .Mode  = GPIO_MODE_OUTPUT_OD,
        .Pull  = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW 
    };
    HAL_GPIO_Init(GPIOC, (GPIO_InitTypeDef *)&GPIO_InitStruct);
}
led::~led(void)
{
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_13);
    __HAL_RCC_GPIOC_CLK_DISABLE();
}
void led::on(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
}
void led::off(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}
void led::toggle(void)
{
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

}; // namespace bsp

extern "C" void SysTick_Handler(void);
void SysTick_Handler(void)
{
    csp::tick::cb_tick_handl();
}
