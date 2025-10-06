#pragma once
#include <common/types.h>

using namespace common;

namespace drivers {

class Driver{
public:
    Driver();
    ~Driver();

    virtual void activate() = 0; // must be implemented
    virtual int reset();
    virtual void deactivate();
};

class DriverManager{
private:
uint8_t m_num_drivers;
public:
    Driver* m_drivers[255]; // TODO move back to private
    DriverManager();
    void add_driver(Driver* driver);
    void activate_all();
};

}