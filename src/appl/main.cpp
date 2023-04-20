
#include <stdlib.h>
#include <stdio.h>

#include "os.h"
#include "bsp.h"

struct kernel: public os::kernel<kernel>
{
    static void hw_init(void)
    {
        csp::tick::init(1, kernel::tick);
    }
    static void sw_init(void){}
    using os::kernel<kernel>::kernel;
};


struct idle: public os::task<0x68, os::priority::idle>
{
    void task_func(void) override;
    using os::task<0x68, os::priority::idle>::task;
};

void idle::task_func(void)
{
    for(;;) csp::halt();
}


struct task: public os::task<0xC0>
{
    void task_func(void) override;
    using os::task<0xC0>::task;
};

void task::task_func(void)
{
    for(bsp::led C13;;sleep(200))
        C13.toggle();
}

int main(void)
{
    printf("\x1B[31mC\x1B[32mO\x1B[33mL\x1B[34mO\x1B[35mR\x1B[42m \x1B[0m \x1B[36mT\x1B[37mE\x1B[30m\x1B[47mS\x1B[0mT\n"); // Color test

    static task task_obj = "user_task";
    static idle idle_obj = "user_idle";    
    static kernel krnl(csp::stack_ptr, csp::stack_size);
}
