
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

//-- task for led blinking ------------------------------------------------------------------------/

static struct task: os::task<task, 0x68>
{
    void task_func(void) __attribute__((__noreturn__))
    {
        for(bsp::led C13;;sleep(200))
            C13.toggle();
    }
    using os::task<task, 0x68>::task;
} task_obj = "blink_task";

//-- tasks for yeld testing -----------------------------------------------------------------------/

#if 1
static struct  dtask: os::task<dtask, 0x58,os::priority::low>
{
    void task_func(void) __attribute__((__noreturn__))
    {
        for(int i = 0;;i++) if (i & 1) yield();
    }
    using os::task<dtask, 0x58, os::priority::low>::task;
} dtask_obj[] = {"dummy_task1", "dummy_task2"};
#endif

//-- task for printf and mutex testing ------------------------------------------------------------/

#if 0
static struct printf_task: os::task<printf_task, 0xF8>
{
    void task_func(void) __attribute__((__noreturn__))
    {
        for(uint32_t i = 0;;i++)
        {
            const char s[] = "-\\|/";
            printf("%d %c\r", os::tick_get(), s[i & 3]);
            
            sleep(500);
        }
    }
    using os::task<printf_task, 0xF8>::task;
} printf_task_obj = "printf_task";
#endif

//-- task for fmem pool testing -------------------------------------------------------------------/

#if 1
static struct fmem_task: os::task<fmem_task, 0x110>
{
    struct item_t
    {
        void *ptr;
        uint32_t reserve;
        uint8_t noalign;
    };

    os::fmempool<item_t, 3> pool_1;
    os::fmempool<item_t, 3> pool_2;
    
    os::fmem<item_t>::item items[6] = {pool_1, pool_1, pool_1, pool_1, pool_1};
    
    void print_snapshot(void)
    {
        printf("snapshot:\n");
        printf("mempool_1 used: %d, free: %d\n", pool_1.used_cnt_get(), pool_1.free_cnt_get());
        printf("mempool_2 used: %d, free: %d\n", pool_2.used_cnt_get(), pool_2.free_cnt_get());
        int cnt = 0;
        for (auto &i : items)
        {
            printf("item %d owner: " U32 " pointer: " U32 "\n", cnt++, i.owner, i.ptr);
        }
        printf("\n");
    }

    void task_func(void) __attribute__((__noreturn__))
    {
        printf("sizeof(item_t) = %d\n", sizeof(item_t));
        print_snapshot();

        items[3].acquire(pool_2, os::nowait);
        items[4].acquire(pool_2, os::nowait);
        items[5].acquire(pool_2, os::nowait);
        print_snapshot();

        items[5].move(pool_1);
        items[4].move(pool_1);
        items[3].move(pool_1);
        print_snapshot();

        for (auto &i : items) i.release();
        print_snapshot();
        
        for(uint32_t i = 0;;i++)
        {
            const char s[] = "-\\|/";
            printf("%d %c\r", os::tick_get(), s[i & 3]);
            
            sleep(500);
        }
    }
    using os::task<fmem_task, 0x110>::task;
} fmem_task_obj = "fmem_task";
#endif

//-- main -----------------------------------------------------------------------------------------/

int main(void)
{
    printf("\x1B[31mC\x1B[32mO\x1B[33mL\x1B[34mO\x1B[35mR\x1B[42m \x1B[0m \x1B[36mT\x1B[37mE\x1B[30m\x1B[47mS\x1B[0mT\n"); // Color test

    static kernel krnl(csp::stack_ptr, csp::stack_size);
}
