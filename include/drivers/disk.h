#pragma once
#include <common/types.h>

using namespace common;

namespace drivers {

enum class DiskErr{
    SUCCESS=0,
    DNE,
    NOT_ATA,
    INVALID_SECTOR,
    INVALID_SIZE,
    NO_SPACE,
    OTHER
};

class Disk {
    /* ==== Working Disk Abstract Class ==== */
    public:
    virtual DiskErr identify() = 0;
    virtual DiskErr read_28(uint32_t sector, uint8_t* data, size_t count) = 0;
    virtual DiskErr write_28(uint32_t sector, uint8_t* data, size_t count) = 0;
    virtual DiskErr flush() = 0;
};

}