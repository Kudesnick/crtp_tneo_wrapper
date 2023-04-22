
#include <stdlib.h>
#include <stdio.h>

#include "os.h"
#include "bsp.h"

struct kernel: public os::kernel<kernel, 0x50>
{
    void hw_init(void)
    {
        csp::tick::init(1, kernel::tick);
    }
    static void sw_init(void)
    {
        kernel::tslice_set(os::priority::low, 10);
    }
    void task_func(void) __attribute__((__noreturn__))
    {
        for(;;)
            csp::halt();
    }
    using os::kernel<kernel, 0x50>::kernel;
};

struct dtask: public os::task<dtask, 0x50,os::priority::low>
{
    void task_func(void) __attribute__((__noreturn__))
    {
        for(;;);
    }
    using os::task<dtask, 0x50, os::priority::low>::task;
};
static dtask dtask_obj[2] = {"dummy_task1", "dummy_task2"};


struct task: public os::task<task, 0xA8>
{
    void task_func(void) __attribute__((__noreturn__))
    {
        for(bsp::led C13;;sleep(200))
            C13.toggle();
    }
    using os::task<task, 0xA8>::task;
};
static task task_obj = "user_task";


int main(void)
{
    printf("\x1B[31mC\x1B[32mO\x1B[33mL\x1B[34mO\x1B[35mR\x1B[42m \x1B[0m \x1B[36mT\x1B[37mE\x1B[30m\x1B[47mS\x1B[0mT\n"); // Color test

    static kernel krnl(csp::stack_ptr, csp::stack_size);
}
