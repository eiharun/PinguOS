#pragma once
#include <common/types.h>

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
    Driver* m_drivers[255];
    uint8_t m_num_drivers;
public:
    DriverManager();
    void add_driver(Driver* driver);
    void activate_all();
};