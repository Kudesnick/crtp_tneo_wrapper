
#include "RTE_Components.h"
#include CMSIS_device_header

#include <stdlib.h>
#include <stdio.h>

#include "os.h"

class kernel: public os::kernel<kernel>
{
public:
    os::stack<0x100> idle_stack;
    os::stack<0x100> int_stack;
    static void init_hw(void);
    static void idle_task(void){}
    static void init_sw(void)
    {
        __NOP();
    }
};

void kernel::init_hw(void)
{
    if (SysTick_Config(HSE_VALUE / 1000)) os::hardfault();
}

extern "C" void SysTick_Handler(void);
void SysTick_Handler(void)
{
    kernel::tick();
}

int main(void)
{
    printf("\x1B[31mC\x1B[32mO\x1B[33mL\x1B[34mO\x1B[35mR\x1B[42m \x1B[0m \x1B[36mT\x1B[37mE\x1B[30m\x1B[47mS\x1B[0mT\n"); // Color test

    static kernel krnl;

    printf("Error");
}
