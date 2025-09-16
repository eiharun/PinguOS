#include <drivers/mouse.h>

MouseHandler::MouseHandler(){}

TextualMouseHandler::TextualMouseHandler(uint32_t init_x, uint32_t init_y, uint8_t scalar)
: m_scalar(scalar), m_x(init_x), m_y(init_y)
{
    m_vga_buf[80*m_y+m_x] = ((m_vga_buf[80*m_y+m_x] & 0xF000) >> 4) 
                        | ((m_vga_buf[80*m_y+m_x] & 0x0F00) << 4) 
                        | (m_vga_buf[80*m_y+m_x] & 0x00FF); // Invert color
}

void TextualMouseHandler::on_move(int8_t x, int8_t y){
    m_vga_buf[80*m_y+m_x] = ((m_vga_buf[80*m_y+m_x] & 0xF000) >> 4) 
                        | ((m_vga_buf[80*m_y+m_x] & 0x0F00) << 4) 
                        | (m_vga_buf[80*m_y+m_x] & 0x00FF); // Invert color
    const uint8_t x_scalar = m_scalar;
    const uint8_t y_scalar = m_scalar;
    // Set x
    m_x += x/x_scalar;
    if( m_x < 0 ){
        m_x=0;
    }
    if(m_x >= 80){
        m_x=79;
    }
    // Set y
    m_y -= y/y_scalar;
    if( m_y < 0 ){
        m_y=0;
    }
    if(m_y >= 25){
        m_y=24;
    }

    m_vga_buf[80*m_y+m_x] = ((m_vga_buf[80*m_y+m_x] & 0xF000) >> 4) 
                    | ((m_vga_buf[80*m_y+m_x] & 0x0F00) << 4) 
                    | (m_vga_buf[80*m_y+m_x] & 0x00FF); // Invert color

}

uint8_t TextualMouseHandler::on_button(uint8_t buttons){
    for(int i=0; i < 3; ++i){
        if( (buttons & (0x01 << i)) != (m_buttons & (0x01 << i))){
            m_vga_buf[80*m_y+m_x] = ((m_vga_buf[80*m_y+m_x] & 0xF000) >> 4) 
                                | ((m_vga_buf[80*m_y+m_x] & 0x0F00) << 4) 
                                | (m_vga_buf[80*m_y+m_x] & 0x00FF);
        }
    }
    m_buttons = buttons;
    return m_buttons;
}

MouseDriver::MouseDriver(InterruptManager* interrupt_manager, MouseHandler* event_handler)
: InterruptHandler(MOUSE_INT, interrupt_manager),
m_handler(event_handler),
m_data_port(0x60),
m_cmd_port(0x64)
{
     
}

void MouseDriver::activate(){
    m_offset = 0;
    m_buttons = 0;
    m_cmd_port.write(0xA8); // activate interrupts
    m_cmd_port.write(0x20); // get current state
    uint8_t status = m_data_port.read() | 2;
    m_cmd_port.write(0x60); // set state
    m_data_port.write(status);
    m_cmd_port.write(0xD4);
    m_data_port.write(0xF4);  
    m_data_port.read(); 
}

MouseDriver::~MouseDriver(){}

uint32_t MouseDriver::handle_interrupt(uint32_t esp){
    uint8_t status = m_cmd_port.read();
    if(!(status & 0x20)){
        return esp;
    }
    
    m_buf[m_offset] = m_data_port.read();
    m_offset = (m_offset + 1) % 3;
    
    if(m_offset==0){
        m_handler->on_move(m_buf[1], m_buf[2]);
        m_buttons = m_handler->on_button(m_buf[0]);
    }

    return esp;
}