#pragma once

#include <stdint.h>
#include "csp.h"

namespace csp
{

namespace led
{
    res init(void);
    void set(const bool _state);
    void toggle(void);
    res deinit(void);
}

}; // namespace csp
