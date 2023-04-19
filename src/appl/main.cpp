
#include <stdlib.h>
#include <stdio.h>

#include "os.h"
#include "bsp.h"

struct kernel: public os::kernel<kernel, 0x68>
{
    static void hw_init(void)
    {
        csp::tick::init(1, kernel::tick);
    }
    static void sw_init(void){}
    void task_func(void) __attribute__((__noreturn__))
    {
        for(;;)
        {
            csp::halt();
        }
    }
    using os::kernel<kernel, 0x68>::kernel;
};

static struct task: public os::task<task, 0xC0>
{
    void task_func(void) __attribute__((__noreturn__))
    {
        for(bsp::led C13;;sleep(200))
            C13.toggle();
    }
    using os::task<task, 0xC0>::task;
} task_obj = "user_task";

int main(void)
{
    printf("\x1B[31mC\x1B[32mO\x1B[33mL\x1B[34mO\x1B[35mR\x1B[42m \x1B[0m \x1B[36mT\x1B[37mE\x1B[30m\x1B[47mS\x1B[0mT\n"); // Color test

    static kernel krnl(csp::stack_ptr, csp::stack_size);
}
