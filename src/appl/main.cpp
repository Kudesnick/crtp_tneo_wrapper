
#include <stdlib.h>

#include "bsp.h"
#include "csp.h"
#include "os.h"
#include "misc.h"
#include "csp_spi.h"
#include "mx25l128.h"

#define TEST_TIMER  (1)
#define TEST_YIELD  (1)
#define TEST_PRINTF (1)
#define TEST_FMEM   (1)
#define TEST_SPI    (1)
#define TEST_MX25   (1)
#define TEST_QUEUE  (1)
#define TEST_FQUEUE (1)
#define TEST_SUSP   (1)

//-- timer test -----------------------------------------------------------------------------------/

#if TEST_TIMER
static struct blink_task: os::task<blink_task, STK(0x70)>
{
    static inline os::semaphore blink_sem = {1,1};
    
    struct timer: os::timer<timer, 200, os::repeat, os::opt::start>
    {
        void timer_func(void)
        {
            blink_sem.release();
        }
    } blink_timer;

    void task_func(void) __attribute__((__noreturn__))
    {
        for(bsp::led C13;;blink_sem.acquire(os::infinitely))
            C13.toggle();
    }
    using os::task<blink_task, STK(0x70)>::task;
} blink_task_obj = "blink_task";
#endif

//-- tasks for yeld testing -----------------------------------------------------------------------/

#if TEST_YIELD
static struct  dtask: os::task<dtask, STK(0x58)>
{
    void task_func(void) __attribute__((__noreturn__))
    {
        for(int i = 0;;i++) if (i & 1) yield();
    }
    using os::task<dtask, STK(0x58)>::task;
} dtask_obj[] =
{
    {"dummy_task1", os::priority::low},
    {"dummy_task2", os::priority::low},
};
#endif

//-- task for printf and mutex testing ------------------------------------------------------------/

#if TEST_PRINTF
static struct printf_task: os::task<printf_task, STK(0xF8)>
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
    using os::task<printf_task, STK(0xF8)>::task;
} printf_task_obj = "printf_task";
#endif

//-- task for fmem pool testing -------------------------------------------------------------------/

#if TEST_FMEM
static struct fmem_task: os::task<fmem_task, STK(0x120)>
{
    struct item_t
    {
        void *ptr;
        uint32_t reserve;
        uint8_t noalign;
    };

    os::fmem<item_t, 3> pool_1;
    os::fmem<item_t, 3> pool_2;
    os::fmem<item_t, 0> pool_3;
    
    item_t raw_pool[3];

    os::fmem_typed<item_t>::item items[6] = {pool_1, pool_1, pool_1, pool_1, pool_1};
    
    void print_snapshot(void)
    {
        printf("snapshot:\n");
        printf("mempool_1 " U32 " used: %d, free: %d\n", &pool_1, pool_1.used_cnt_get(), pool_1.free_cnt_get());
        printf("mempool_2 " U32 " used: %d, free: %d\n", &pool_2, pool_2.used_cnt_get(), pool_2.free_cnt_get());
        printf("mempool_3 " U32 " used: %d, free: %d\n", &pool_3, pool_3.used_cnt_get(), pool_3.free_cnt_get());
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
        printf("After constructor");
        print_snapshot();

        items[3].acquire(pool_2, os::infinitely);
        items[4].acquire(pool_2, os::infinitely);
        items[5].acquire(pool_2, os::infinitely);
        printf("After acure in pool_2 ");
        print_snapshot();

        items[5].move(pool_1);
        items[4].move(pool_1);
        items[3].move(pool_1);
        printf("After move in pool_1 ");
        print_snapshot();

        for (auto &i : items) i.release();
        printf("After release all ");
        print_snapshot();

        for (auto &i : raw_pool) pool_2.append(i);
        printf("After append raw_pool to pool_2 ");
        print_snapshot();
        
        for (uint32_t i = 0; i < countof(raw_pool); i++)
        {
            items[i].acquire(pool_2, os::infinitely);
        }
        printf("After accure raw_pool ");
        print_snapshot();
        
        for(uint32_t i = 0;;i++)
        {
            const char s[] = "-\\|/";
            printf("%d %c\r", os::tick_get(), s[i & 3]);
            
            sleep(500);
        }
    }
    using os::task<fmem_task, STK(0x120)>::task;
} fmem_task_obj = "fmem_task";
#endif

//-- task for SPI testing -------------------------------------------------------------------------/

#if TEST_SPI
static struct spi_task: os::task<spi_task, STK(0x68)>
{
    static inline uint8_t RES[]  = {0xAB, 0x00, 0x00, 0x00, 0x00};
    static inline uint8_t RDID[] = {0x9F, 0x00, 0x00, 0x00};
    static inline uint8_t REMS[] = {0x90, 0x00, 0x00, 0x00, 0x00, 0x00};

    void task_func(void)
    {
        csp::spi::init();
        sleep(100);
        csp::spi::cs_on();
        csp::spi::send(RES, sizeof(RES), RES);
        sleep(100);
        csp::spi::cs_off();
        csp::spi::cs_on();
        csp::spi::send(RDID, sizeof(RDID), RDID);
        sleep(100);
        csp::spi::cs_off();
        csp::spi::cs_on();
        csp::spi::send(REMS, sizeof(REMS), REMS);
        sleep(100);
        csp::spi::cs_off();
    }
    using os::task<spi_task, STK(0x68)>::task;
} spi_task_obj = "spi_task";
#endif

//-- task for mx25l128 testing --------------------------------------------------------------------/

#if TEST_MX25
static struct mx25_task: os::task<mx25_task, STK(0xF8)>
{
private:
    bsp::mx25 flash;
public:    
    void task_func(void)
    {
        sleep(1_sec);

        flash.reset();
        bsp::mx25::id id;
        flash.read_id(id);
        
        printf("manufacturer id: 0x%X, type: 0x%X, density: 0x%X\n", id.manufacturer_id, id.type, id.density);
    }
    using os::task<mx25_task, STK(0xF8)>::task;
} mx25_task_obj = "mx25_task";
#endif

//-- task for queue testing -----------------------------------------------------------------------/

#if TEST_QUEUE
static struct queue_task: os::task<queue_task, STK(0xF0)>
{
    os::queue<uint32_t, 4> small_queue;
    
    void task_func(void) __attribute__((__noreturn__))
    {
        for(;;)
        {
            sleep(1000);
            
            for (uint32_t i = 0; i < 6; i++, sleep(100))
            {
                auto res = small_queue.send(i, 300);
                switch(res)
                {
                    case os::rc::ok: printf("%d value sended to small_queue\n", i); break;
                    case os::rc::timeout: printf("%d value not sended to small_queue (timeout)\n", i); break;
                    default: printf("Test failed! %d value not sended to small_queue (something wrong)\n", i); break;
                }
            }
            
            small_queue.reset();
            printf("small_queue is reset\n");
            
            for (uint32_t i = 0; i < 6; i++, sleep(100))
            {
                auto res = small_queue.send(i, 300);
                switch(res)
                {
                    case os::rc::ok: printf("%d value sended to small_queue\n", i); break;
                    case os::rc::timeout: printf("%d value not sended to small_queue (timeout)\n", i); break;
                    default: printf("Test failed! %d value not sended to small_queue (something wrong)\n", i); break;
                }
            }
            
            for (uint32_t i = 0; i < 6; i++, sleep(100))
            {
                uint32_t tmp;
                auto res = small_queue.receive(tmp, 300);
                switch(res)
                {
                    case os::rc::ok: printf("%d value received from small_queue\n", tmp); break;
                    case os::rc::timeout: printf("Value not received from small_queue (timeout)\n"); break;
                    default: printf("Test failed! Value not received from small_queue (something wrong)\n", i); break;
                }
            }
        }
    }
    using os::task<queue_task, STK(0xF0)>::task;
} queue_task_obj = "queue_task";
#endif

//-- task for fmem_queue testing ------------------------------------------------------------------/

#if TEST_FQUEUE
static struct fqueue_task: os::task<fqueue_task, STK(0xF8)>
{
    struct __attribute__((packed, aligned(1))) msg_t
    {
        uint32_t f0, f1, f2;
        uint8_t noalign_field;
    };
    
    os::fmem_queue<msg_t, 4, 4> large_queue;
    os::fmem_queue<msg_t, 0, 0> empty_large_queue;
    
    void task_func(void) __attribute__((__noreturn__))
    {
        for(;;)
        {
            sleep(1000);
            
            printf("msg_t size = %d\n", sizeof(msg_t));
            
            for (uint32_t i = 1; i < 7; i++, sleep(100))
            {
                msg_t msg = {i, i << 1, i << 2, static_cast<typeof(msg_t::noalign_field)>(i)};
                auto res = large_queue.send(msg, 300);
                switch(res)
                {
                    case os::rc::ok: printf("{%d, %d, %d} value sended to extended_queue\n", msg.f0, msg.f1, msg.f2); break;
                    case os::rc::timeout: printf("{%d, %d, %d} value not sended to extended_queue (timeout)\n", msg.f0, msg.f1, msg.f2); break;
                    default: printf("Test failed! {%d, %d, %d} value not sended to extended_queue (something wrong)\n", msg.f0, msg.f1, msg.f2); break;
                }
            }
            
            large_queue.reset();
            printf("large_queue is reset\n");
            
            for (uint32_t i = 1; i < 7; i++, sleep(100))
            {
                msg_t msg = {i, (i << 1) + 0xF0, (i << 2) + 0xF0, static_cast<typeof(msg_t::noalign_field)>(i)};
                auto res = large_queue.send(msg, 300);
                switch(res)
                {
                    case os::rc::ok: printf("{%d, %d, %d} value sended to extended_queue\n", msg.f0, msg.f1, msg.f2); break;
                    case os::rc::timeout: printf("{%d, %d, %d} value not sended to extended_queue (timeout)\n", msg.f0, msg.f1, msg.f2); break;
                    default: printf("Test failed! {%d, %d, %d} value not sended to extended_queue (something wrong)\n", msg.f0, msg.f1, msg.f2); break;
                }
            }
            
            for (uint32_t i = 0; i < 6; i++, sleep(100))
            {
                msg_t msg;
                auto res = large_queue.receive(msg, 300);
                switch(res)
                {
                    case os::rc::ok: printf("{%d, %d, %d} value received from extended_queue\n", msg.f0, msg.f1, msg.f2); break;
                    case os::rc::timeout: printf("Value not received from extended_queue (timeout)\n"); break;
                    default: printf("Test failed! Value not received from extended_queue (something wrong)\n", i); break;
                }
            }
        }
    }
    using os::task<fqueue_task, STK(0xF8)>::task;
} fqueue_task_obj = "fqueue_task";
#endif

//-- suspended task -------------------------------------------------------------------------------/ 

#if TEST_SUSP
static struct susp_ctl_task : os::task<susp_ctl_task, STK(0x60)>
{
    struct susp_task : os::task<susp_task, STK(0xC0)>
    {
        void task_func(void)
        {
            printf("susp_task is runing\n");
        }
        using os::task<susp_task, STK(0xC0)>::task;
    } susp_task_obj = {"susp_task", os::priority::normal, os::opt::nostart};

    void task_func(void) __attribute__((__noreturn__))
    {
        for (;;sleep(1000))
        {
            susp_task_obj.activate();
        }
    }
    using os::task<susp_ctl_task, STK(0x60)>::task;
} susp_ctl_task_obj = "susp_ctl_task";
#endif

//-- init callbacks -------------------------------------------------------------------------------/

#if TN_DYNAMIC_TICK

static uint32_t tick_shedule;

void csp::tick::cb_tick_handl(void)
{
    if (csp::tick::tick_get() >= tick_shedule)
    {
        os::tick();
    }
}

void os::sheduler::cb_sleep_until(uint32_t _timestamp)
{
    tick_shedule = _timestamp;
}

uint32_t os::sheduler::cb_tick_get(void)
{
    return csp::tick::tick_get();
}

#else

void csp::tick::cb_tick_handl(void)
{
    os::tick();
}

#endif

//-- kernel object --------------------------------------------------------------------------------/

struct kernel: os::kernel<kernel, STK(0x48)>
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
    using os::kernel<kernel, STK(0x48)>::kernel;
};

//-- main -----------------------------------------------------------------------------------------/

int main(void)
{
    printf("\x1B[31mC\x1B[32mO\x1B[33mL\x1B[34mO\x1B[35mR\x1B[42m \x1B[0m \x1B[36mT\x1B[37mE\x1B[30m\x1B[47mS\x1B[0mT\n"); // Color test

    static kernel krnl(csp::stack_ptr, csp::stack_size);
}
