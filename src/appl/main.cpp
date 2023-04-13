
#include <stdlib.h>
#include <stdio.h>

#include "os.h"
#include "bsp.h"

class kernel: public os::kernel<kernel>
{
public:
    os::stack<0x58> idle_stack;
    static void init_hw(void);
    static void idle_task(void);
    static void init_sw(void);
};

class task: public os::task<task>
{
public:
    os::stack<0xC0> stack;
    void task_func(void) __attribute__((__noreturn__))
    {
        for(bsp::led C13;;sleep(500))
            C13.toggle();
    }
};

void kernel::init_hw(void)
{
    csp::tick::init(1, kernel::tick);
}

void kernel::idle_task(void)
{
    csp::halt();
}

void kernel::init_sw(void)
{
    static task tsk;
}

int main(void)
{
    printf("\x1B[31mC\x1B[32mO\x1B[33mL\x1B[34mO\x1B[35mR\x1B[42m \x1B[0m \x1B[36mT\x1B[37mE\x1B[30m\x1B[47mS\x1B[0mT\n"); // Color test

    static kernel krnl;

    printf("Error");
}
