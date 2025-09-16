
#pragma once
#include <common/types.h>
#include <hardware_communication/interrupts.h>
#include <hardware_communication/port.h>
#include <drivers/driver.h>

class MouseDriver: public InterruptHandler, public Driver{    
public:
    MouseDriver(InterruptManager* interrupt_manger);
    ~MouseDriver();
    virtual uint32_t handle_interrupt(uint32_t esp) override;
    virtual void activate() override;
    
private:
    Port8 m_data_port;
    Port8 m_cmd_port;

    int8_t m_buf[3];
    uint8_t m_offset;
    uint8_t m_buttons;
    
};

#define MOUSE_INT 0x2C