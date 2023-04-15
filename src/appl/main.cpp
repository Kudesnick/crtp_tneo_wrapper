
#include <stdlib.h>
#include <stdio.h>

#include "os.h"
#include "bsp.h"

struct kernel: public os::kernel
{
    os::stack<0x58> idle_stack;
    static void init_hw(void);
    static void idle_task(void);
    static void init_sw(void);

    kernel(): os::kernel(this){}
};

static struct task: public os::task
{
    os::stack<0xC0> stack;
    static void task_func(void *arg) __attribute__((__noreturn__))
    {
        (void)arg;
        
        for(bsp::led C13;;sleep(500))
            C13.toggle();
    }
    task(void): os::task(this, os::priority::normal, "user_task"){}
} task_obj;

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

}

int main(void)
{
    printf("\x1B[31mC\x1B[32mO\x1B[33mL\x1B[34mO\x1B[35mR\x1B[42m \x1B[0m \x1B[36mT\x1B[37mE\x1B[30m\x1B[47mS\x1B[0mT\n"); // Color test

    static kernel krnl;
}
