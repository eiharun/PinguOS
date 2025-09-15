
#pragma once
#include "types.h"
#include "interrupts.h"
#include "port.h"

class MouseDriver: public InterruptHandler{    
public:
    MouseDriver(InterruptManager* interrupt_manger);
    ~MouseDriver();
    virtual uint32_t handle_interrupt(uint32_t esp) override;
    
private:
    Port8 data_port;
    Port8 cmd_port;

    int8_t buf[3];
    uint8_t offset;
    uint8_t buttons;
    
};

#define MOUSE_INT 0x2C