#include "RTE_Components.h"
#include CMSIS_device_header

#include "csp.h"
#include "bsp.h"
#include "misc.h"

extern "C" const uint32_t Image$$RW_STACK$$Base;
extern "C" const uint32_t Image$$RW_STACK$$Length;

static void dummy(void) {};

namespace csp //------------------------------------------------------------------------------------
{

uint32_t *const &stack_ptr = const_cast<uint32_t *>(&Image$$RW_STACK$$Base);
const uint32_t stack_size = Image$$RW_STACK$$Length - reinterpret_cast<uint32_t>(&Image$$RW_STACK$$Base);

namespace tick //-----------------------------------------------------------------------------------
{
    
static uint32_t tick_ctn;

res init(const uint32_t _ms)
{
    auto res = res::ok;
    
    if (SysTick_Config(HSE_VALUE * _ms / 1000) != 0)
    {
        res = res::err;
        PRINTFAULT("value %d not corrected for systick interval\n", _ms);
    }
    return res;
}

uint32_t tick_get(void)
{
    return tick_ctn;
}

__WEAK void cb_tick_handl(void) {};

}; // namespace tick -------------------------------------------------------------------------------

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

}; // namespace csp --------------------------------------------------------------------------------

// interrupt vectros -------------------------------------------------------------------------------

extern "C" void SysTick_Handler(void);
void SysTick_Handler(void)
{
    csp::tick::tick_ctn++;
    csp::tick::cb_tick_handl();
}
