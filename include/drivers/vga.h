#pragma once
#include <common/types.h>
#include <hardware_communication/port.h>
#include <drivers/driver.h>

using namespace common;
using namespace hardware_communication;

namespace drivers{

class VideoGraphicsArray{
protected:
    Port8 m_misc_port;
    Port8 m_crtc_index_port;
    Port8 m_crtc_data_port;
    Port8 m_sequencer_index_port;
    Port8 m_sequencer_data_port;
    Port8 m_graphics_controller_index_port;
    Port8 m_graphics_controller_data_port;
    Port8 m_attribute_controller_index_port;
    Port8 m_attribute_controller_read_port;
    Port8 m_attribute_controller_write_port;
    Port8 m_attribute_controller_reset_port;

    void write_registers(uint8_t* registers);
    uint8_t* get_frame_buffer_segment(); 
    virtual void put_pixel(uint32_t x, uint32_t y, uint8_t color_index);
    virtual uint8_t get_color_index(uint8_t r, uint8_t g, uint8_t b);
public:
    VideoGraphicsArray();
    ~VideoGraphicsArray();
    virtual bool supports_mode(uint32_t width, uint32_t height, uint32_t color_depth);
    virtual bool set_mode(uint32_t width, uint32_t height, uint32_t color_depth);
    virtual void put_pixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b);
};




}