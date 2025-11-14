#pragma once
#include<common/types.h>

using namespace common;

namespace drivers {

template <class Err>
class Disk {
    /* ==== Working Disk Abstract Class ==== */
    public:
    virtual Err identify() = 0;
    virtual Err read_28(uint32_t sector, uint8_t* data, size_t count) = 0;
    virtual Err write_28(uint32_t sector, uint8_t* data, size_t count) = 0;
    virtual Err flush() = 0;
};

}