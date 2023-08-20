#include "mx25l128.h"
#include "csp_spi.h"

using namespace bsp;

res mx25::reset(const cmd _reset_for)
{
    if (mutex.acquire(os::nowait) != os::rc::ok) return res::err;

    csp::spi::cs_on();
    csp::spi::send(RSTEN);
    os::task_base::sleep(2); /// @todo
    csp::spi::cs_off();
    os::task_base::sleep(1); // tSHSL < 30 ns
    csp::spi::cs_on();
    csp::spi::send(RST);
    os::task_base::sleep(2); /// @todo
    csp::spi::cs_off();
    switch (_reset_for) // tREADY2
    {
        case SE    : os::task_base::sleep( 12); break;
        case BE_32K: os::task_base::sleep( 25); break;
        case BE    : os::task_base::sleep( 25); break;
        case CE    : os::task_base::sleep(100); break;
        case WRSR  : os::task_base::sleep( 40); break;
        default    : os::task_base::sleep(  1); break;
    }

    return mutex.release() == os::rc::ok ? res::ok : res::err;
}

res mx25::read_id(mx25::id &_id)
{
    if (mutex.acquire(os::nowait) != os::rc::ok) return res::err;
    
    csp::spi::cs_on();
    csp::spi::send(RDID);
    os::task_base::sleep(2); /// @todo
    csp::spi::read(reinterpret_cast<uint8_t *>(&_id), sizeof(_id));
    os::task_base::sleep(2); /// @todo
    csp::spi::cs_off();
    
    return mutex.release() == os::rc::ok ? res::ok : res::err;
}

mx25::mx25(void)
{
    csp::spi::init();
}

mx25::~mx25()
{
    csp::spi::deinit();
}
