#pragma once
#include <common/types.h>
#include <hardware_communication/interrupts.h>
#include <hardware_communication/port.h>
#include <drivers/driver.h>

using namespace common;
using namespace hardware_communication;

namespace drivers {

class MouseHandler{
public:
    MouseHandler();

    virtual void on_move(int8_t x, int8_t y) = 0;
    virtual uint8_t on_button(uint8_t buttons) = 0;
};

class TextualMouseHandler: public MouseHandler {
public:
    TextualMouseHandler(uint32_t init_x, uint32_t init_y, uint8_t sens);

    void on_move(int8_t x, int8_t y) override;
    uint8_t on_button(uint8_t buttons) override;
private:
    uint16_t* m_vga_buf = (uint16_t*) 0xB8000;
    const uint8_t m_sens;
    int32_t m_x;
    int32_t m_y;
    int32_t m_accum_x{}; // Accumulation of small movements
    int32_t m_accum_y{};
    uint8_t m_buttons;
};

class MouseDriver: public InterruptHandler, public Driver{    
public:
    MouseDriver(InterruptManager* interrupt_manger, MouseHandler* event_handler);
    ~MouseDriver();
    virtual uint32_t handle_interrupt(uint32_t esp) override;
    virtual void activate() override;
    
private:
    MouseHandler* m_handler;

    Port8 m_data_port;
    Port8 m_cmd_port;

    int8_t m_buf[3];
    uint8_t m_offset;
    uint8_t m_buttons;
    
};

#define MOUSE_INT 0x2C

}
