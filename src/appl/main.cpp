
#include <stdlib.h>
#include <stdio.h>

#include "bsp.h"
#include "os.h"

struct kernel: os::kernel<kernel, 0x50>
{
    void hw_init(void)
    {
        csp::tick::init(1, os::tick);
    }
    static void sw_init(void)
    {
        os::tslice_set(os::priority::low, 10);
    }
    void task_func(void) __attribute__((__noreturn__))
    {
        for(;;)
            csp::halt();
    }
    using os::kernel<kernel, 0x50>::kernel;
};

static struct  dtask1: os::task<dtask1, 0xA8,os::priority::low>
{
    void task_func(void) __attribute__((__noreturn__))
    {
        for(int i = 0;;i++) if (i & 1) yield();
    }
    using os::task<dtask1, 0xA8, os::priority::low>::task;
} dtask1_obj = "dummy_task1";

static struct dtask2: os::task<dtask2, 0xA8,os::priority::low>
{
    void task_func(void) __attribute__((__noreturn__))
    {
        for(int i = 0;;i++) if (i & 1) yield();
    }
    using os::task<dtask2, 0xA8, os::priority::low>::task;
} dtask2_obj = "dummy_task2";

static struct task: os::task<task, 0xA8>
{
    void task_func(void) __attribute__((__noreturn__))
    {
        for(bsp::led C13;;sleep(200))
            C13.toggle();
    }
    using os::task<task, 0xA8>::task;
} task_obj = "user_task";


int main(void)
{
    printf("\x1B[31mC\x1B[32mO\x1B[33mL\x1B[34mO\x1B[35mR\x1B[42m \x1B[0m \x1B[36mT\x1B[37mE\x1B[30m\x1B[47mS\x1B[0mT\n"); // Color test

    static kernel krnl(csp::stack_ptr, csp::stack_size);
}
