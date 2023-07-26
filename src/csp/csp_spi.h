#pragma once

#include <stdint.h>
#include "csp.h"

namespace csp
{

namespace spi
{
    res init(void);
    res deinit(void);
    void cs_on(void);
    void cs_off(void);

    res send(const uint8_t _source[], const uint32_t _len, uint8_t *const _dest = nullptr);
    res read(const uint8_t _dummy, uint8_t _dest[], const uint32_t _len);

    void cb_complete(uint8_t *const _dest, const uint32_t _dest_len); // weak
}

}; // namespace csp
