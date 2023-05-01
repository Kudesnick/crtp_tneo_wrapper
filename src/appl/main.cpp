
#include <stdlib.h>

#include "bsp.h"
#include "os.h"
#include "misc.h"

//-- init callbacks -------------------------------------------------------------------------------/

void csp::tick::cb_tick_handl(void)
{
    os::tick();
}

//-- kernel object --------------------------------------------------------------------------------/

struct kernel: os::kernel<kernel, 0x48>
{
    void hw_init(void)
    {
        csp::tick::init(1);
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
    using os::kernel<kernel, 0x48>::kernel;
};

//-- tasks for yeld testing -----------------------------------------------------------------------/

static struct  dtask1: os::task<dtask1, 0x78,os::priority::low>
{
    void task_func(void) __attribute__((__noreturn__))
    {
        for(int i = 0;;i++) if (i & 1) yield();
    }
    using os::task<dtask1, 0x78, os::priority::low>::task;
} dtask1_obj = "dummy_task1";

static struct dtask2: os::task<dtask2, 0x78,os::priority::low>
{
    void task_func(void) __attribute__((__noreturn__))
    {
        for(int i = 0;;i++) if (i & 1) yield();
    }
    using os::task<dtask2, 0x78, os::priority::low>::task;
} dtask2_obj = "dummy_task2";

//-- task for led blinking ------------------------------------------------------------------------/

static struct task: os::task<task, 0xA0>
{
    void task_func(void) __attribute__((__noreturn__))
    {
        for(bsp::led C13;;sleep(200))
            C13.toggle();
    }
    using os::task<task, 0xA0>::task;
} task_obj = "blink_task";

//-- task for printf and mutex testing ------------------------------------------------------------/

#if 1
static struct printf_task: os::task<printf_task, 0x240>
{
    void task_func(void) __attribute__((__noreturn__))
    {
        for(uint32_t i = 0;;i++)
        {
            static const char s[] = "-\\|/";
            printf("%d %c\r", os::tick_get(), s[i & 3]);
            
            sleep(500);
        }
    }
    using os::task<printf_task, 0x240>::task;
} printf_task_obj = "printf_task";
#endif

//-- main -----------------------------------------------------------------------------------------/

int main(void)
{
    printf("\x1B[31mC\x1B[32mO\x1B[33mL\x1B[34mO\x1B[35mR\x1B[42m \x1B[0m \x1B[36mT\x1B[37mE\x1B[30m\x1B[47mS\x1B[0mT\n"); // Color test

    static kernel krnl(csp::stack_ptr, csp::stack_size);
}
