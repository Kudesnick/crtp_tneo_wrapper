#include "mx25l128.h"
#include "csp_spi.h"

using namespace bsp;

void mx25::send_wait(const uint32_t _tout)
{
    waitevnt.wait(event_pattern, os::eventgrp::wait_mode::w_and_clr, _tout);
}

void mx25::send_cmd(const cmd _cmd)
{
    csp::spi::send(reinterpret_cast<const uint8_t *>(&_cmd), 1);
}

res mx25::reset(const cmd _reset_for)
{
    if (mutex.acquire(os::nowait) != os::rc::ok) return res::err;
    waitevnt.clr(event_pattern);

    csp::spi::cs_on();
    send_cmd(cmd::RSTEN);
    send_wait();
    csp::spi::cs_off();
    os::task_base::sleep(2); // tSHSL < 30 ns
    csp::spi::cs_on();
    send_cmd(cmd::RST);
    send_wait();
    csp::spi::cs_off();
    switch (_reset_for) // tREADY2
    {
        case cmd::SE    : os::task_base::sleep( 12); break;
        case cmd::BE_32K: os::task_base::sleep( 25); break;
        case cmd::BE    : os::task_base::sleep( 25); break;
        case cmd::CE    : os::task_base::sleep(100); break;
        case cmd::WRSR  : os::task_base::sleep( 40); break;
        default: os::task_base::sleep(2); break;
    }

    return mutex.release() == os::rc::ok ? res::ok : res::err;
}

res mx25::read_id(mx25::id &_id)
{
    if (mutex.acquire(os::nowait) != os::rc::ok) return res::err;
    waitevnt.clr(event_pattern);
    
    csp::spi::cs_on();
    send_cmd(cmd::RDID);
    send_wait();
    csp::spi::read(reinterpret_cast<uint8_t *>(&_id), sizeof(_id));
    send_wait();
    csp::spi::cs_off();

    return mutex.release() == os::rc::ok ? res::ok : res::err;
}

mx25::mx25(const uint32_t _evnt_pattern): event_pattern(_evnt_pattern)
{
    csp::spi::init();
}

mx25::~mx25()
{
    csp::spi::deinit();
}


void csp::spi::cb_complete(void) // override weak function
{
    mx25::waitevnt.set(~0U);
}
