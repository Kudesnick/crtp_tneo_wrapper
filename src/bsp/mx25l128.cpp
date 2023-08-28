#include "mx25l128.h"
#include "csp_spi.h"

using namespace bsp;

res mx25::send_wait(const uint32_t _tout)
{
    return waitevnt.wait(event_pattern, os::eventgrp::wait_mode::w_and_clr, _tout) == os::rc::ok ? res::ok : res::err;
}

res mx25::send_cmd(const cmd _cmd)
{
    return csp::spi::send(reinterpret_cast<const uint8_t *>(&_cmd), 1);
}

void mx25::concat_addr(uint8_t _cmd[4], const uint32_t _addr)
{
    _cmd[1] = static_cast<uint8_t>(_addr >> 16);
    _cmd[2] = static_cast<uint8_t>(_addr >> 8);
    _cmd[3] = static_cast<uint8_t>(_addr);
}

res mx25::begin(void)
{
    result_ = (true
               && mutex.acquire(os::nowait) == os::rc::ok
               && waitevnt.clr(event_pattern) == os::rc::ok
               && csp::spi::cs_on() == res::ok
               ) ? res::ok : res::err;
    return result_;
}

res mx25::end(void)
{
    csp::spi::cs_off();
    return (true
            && mutex.release() == os::rc::ok
            && result_ == res::ok
            ) ? res::ok : res::err;
}

res mx25::reset(const cmd _reset_for)
{
    if (true
        && begin() == res::ok
        && send_cmd(cmd::RSTEN) == res::ok
        && send_wait() == res::ok
        && csp::spi::cs_off() == res::ok
        && os::task_base::sleep(2) == os::rc::ok // tSHSL < 30 ns
        && csp::spi::cs_on() == res::ok
        && send_cmd(cmd::RST) == res::ok
        && send_wait() == res::ok
        && csp::spi::cs_off() == res::ok
        )
    {
        uint32_t tmp = 2;
        switch (_reset_for) // tREADY2
        {
            case cmd::SE    : tmp =  12; break;
            case cmd::BE_32K: tmp =  25; break;
            case cmd::BE    : tmp =  25; break;
            case cmd::CE    : tmp = 100; break;
            case cmd::WRSR  : tmp =  40; break;
            default: tmp = 2;
        }
        result_ = os::task_base::sleep(tmp) == os::rc::ok ? res::ok : res::err;
    }
    return end();
}

res mx25::read_id(mx25::id &_id)
{
    if (true
        && begin() == res::ok
        && send_cmd(cmd::RDID) == res::ok
        && send_wait() == res::ok
        && csp::spi::read(reinterpret_cast<uint8_t *>(&_id), sizeof(_id)) == res::ok
        && send_wait() == res::ok
    )
    {
        result_ = res::ok;
    }

    return end();
}

res mx25::read_sr(mx25::sr &_sr)
{
    uint8_t buf[2] = {static_cast<uint8_t>(cmd::RDSR), 0};

    if (true
        && begin() == res::ok
        && csp::spi::send(buf, sizeof(buf), buf) == res::ok
        && send_wait() == res::ok
        )
    {
        result_ = res::ok;
        _sr = *reinterpret_cast<mx25::sr *>(&buf[1]);
    }
    return end();
}

res mx25::read_cr(mx25::cr &_cr)
{
    uint8_t buf[2] = {static_cast<uint8_t>(cmd::RDCR), 0};

    if (true
        && begin() == res::ok
        && csp::spi::send(buf, sizeof(buf), buf) == res::ok
        && send_wait() == res::ok
        )
    {
        result_ = res::ok;
        _cr = *reinterpret_cast<mx25::cr *>(&buf[1]);
    }
    return end();
}

res mx25::read(const uint32_t _addr, uint8_t *const _buf, const uint32_t _len)
{
    uint8_t cmd[4] = {static_cast<uint8_t>(cmd::READ)};
    concat_addr(cmd, _addr);

    if (true
        && begin() == res::ok
        && csp::spi::send(cmd, sizeof(cmd)) == res::ok
        && send_wait() == res::ok
        && csp::spi::read(_buf, _len) == res::ok
        && send_wait() == res::ok
        )
    {
        result_ = res::ok;
    }

    return end();
}

res mx25::write(const uint32_t _addr, uint8_t *const _buf, const uint8_t _len)
{
    static constexpr auto bpt_max = 30; // Byte Program Time (via page program command) [us, max]
    uint8_t cmd[4] = {static_cast<uint8_t>(cmd::PP)};
    concat_addr(cmd, _addr);
    
    if (true
        && begin() == res::ok
        && csp::spi::send(cmd, sizeof(cmd)) == res::ok
        && send_wait() == res::ok
        && csp::spi::send(_buf, _len) == res::ok
        && send_wait() == res::ok
        && csp::spi::cs_off() == res::ok
        )
    {
        result_ = res::ok;
    }

    return end();
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
