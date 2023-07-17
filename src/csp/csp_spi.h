#pragma once

#include <stdint.h>
#include "csp.h"

namespace csp
{

namespace spi
{
    res init(void);
    res send(uint8_t *const _buf, const uint32_t _len);
    void cb_send(uint8_t *const _buf, const uint32_t _len); // weak
    res deinit(void);
}

}; // namespace csp