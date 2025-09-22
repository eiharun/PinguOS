#include <drivers/vga.h>

using namespace drivers;

VideoGraphicsArray::VideoGraphicsArray():
m_misc_port(0x3C2),
m_crtc_index_port(0x3D4),
m_crtc_data_port(0x3D5),
m_sequencer_index_port(0x3C4),
m_sequencer_data_port(0x3C5),
m_graphics_controller_index_port(0x3CE),
m_graphics_controller_data_port(0x3CF),
m_attribute_controller_index_port(0x3C0),
m_attribute_controller_read_port(0x3C1),
m_attribute_controller_write_port(0x3C0),
m_attribute_controller_reset_port(0x3DA)
{}

VideoGraphicsArray::~VideoGraphicsArray(){}

void VideoGraphicsArray::write_registers(uint8_t* registers){
    m_misc_port.write(*(registers++));

    for(uint8_t i=0; i<5; ++i){
        m_sequencer_index_port.write(i);
        m_sequencer_data_port.write(*(registers++));
    }

    m_crtc_index_port.write(0x03);
    m_crtc_data_port.write(m_crtc_data_port.read() | 0x80);
    m_crtc_index_port.write(0x11);
    m_crtc_data_port.write(m_crtc_data_port.read() & ~0x80);
    registers[0x03] = registers[0x03] | 0x80;
    registers[0x11] = registers[0x11] & ~0x80;
    for(uint8_t i=0; i<25; ++i){
        m_crtc_index_port.write(i);
        m_crtc_data_port.write(*(registers++));
    }

    for(uint8_t i=0; i<9; ++i){
        m_graphics_controller_index_port.write(i);
        m_graphics_controller_data_port.write(*(registers++));
    }

    for(uint8_t i=0; i<21; ++i){
        m_attribute_controller_read_port.read();
        m_attribute_controller_index_port.write(i);
        m_attribute_controller_write_port.write(*(registers++));
    }
    m_attribute_controller_reset_port.read();
    m_attribute_controller_write_port.write(0x20);
}

bool VideoGraphicsArray::supports_mode(uint32_t width, uint32_t height, uint32_t color_depth){
    return width == 320 && height == 200 && color_depth == 8; // TODO Add more modes later
}

bool VideoGraphicsArray::set_mode(uint32_t width, uint32_t height, uint32_t color_depth){
    if(!supports_mode(width, height, color_depth)){
        return false;
    }
    //Only 320x200 8bit color depth is supported
    uint8_t g_320x200x256[] =
    {
        /* MISC */
            0x63,
        /* SEQ */
            0x03, 0x01, 0x0F, 0x00, 0x0E,
        /* CRTC */
            0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
            0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
            0xFF,
        /* GC */
            0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
            0xFF,
        /* AC */
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
            0x41, 0x00, 0x0F, 0x00, 0x00
    };
    write_registers(g_320x200x256);
    return true;
}

uint8_t* VideoGraphicsArray::get_frame_buffer_segment(){
    m_graphics_controller_index_port.write(0x06);
    uint8_t segment_num = ((m_graphics_controller_data_port.read() >> 2 ) & 0x03);
    switch(segment_num){
        default:
        case 0: return (uint8_t*)0x00000;
        case 1: return (uint8_t*)0xA0000;
        case 2: return (uint8_t*)0xB0000;
        case 3: return (uint8_t*)0xB8000;
    }
}

void VideoGraphicsArray::put_pixel(uint32_t x, uint32_t y, uint8_t color_index){
    uint8_t* pixel_addr = get_frame_buffer_segment() + 320*y + x;
    *pixel_addr = color_index;
}

uint8_t VideoGraphicsArray::get_color_index(uint8_t r, uint8_t g, uint8_t b){
    if(r == 0x00 && g == 0x00 & b == 0xA8){
        return 0x01;
    }
}

void VideoGraphicsArray::put_pixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b){
    put_pixel(x,y, get_color_index(r, g, b));
}