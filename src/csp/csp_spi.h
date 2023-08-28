#pragma once

#include <stdint.h>
#include "csp.h"

namespace csp
{

namespace spi
{
    res init(void);
    res deinit(void);
    res cs_on(void);
    res cs_off(void);

    res send(const uint8_t *const _source, const uint32_t _len, uint8_t *const _dest = nullptr);
    res read(uint8_t *const _dest, const uint32_t _len, const uint8_t _dummy = 0);

    void cb_complete(void); // weak
}

}; // namespace csp
