#pragma once
#include <common/types.h>

using namespace common;

namespace hardware_communication {

class Port {
protected:
    uint16_t m_portnumber;
    Port(uint16_t portnumber);
    ~Port();
};

class Port8: public Port{
public:
    Port8(uint16_t portnumber);
    ~Port8();
    virtual void write(uint8_t data);
    uint8_t read();
};

class Port8_Slow: public Port8{
public:
    Port8_Slow(uint16_t portnumber);
    ~Port8_Slow();
    void write(uint8_t data) override;
};


class Port16: public Port{
public:
    Port16(uint16_t portnumber);
    ~Port16();
    virtual void write(uint16_t data);
    uint16_t read();
};

class Port32: public Port{
public:
    Port32(uint16_t portnumber);
    ~Port32();
    virtual void write(uint32_t data);
    uint32_t read();
};

}
