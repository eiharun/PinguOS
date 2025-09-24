#include <drivers/mouse.h>

using namespace drivers;

MouseHandler::MouseHandler(){}
uint8_t MouseHandler::on_button(uint8_t buttons){}
void MouseHandler::on_up(uint8_t buttons){}
void MouseHandler::on_down(uint8_t buttons){}

TextualMouseHandler::TextualMouseHandler(uint32_t init_x, uint32_t init_y, uint8_t scalar)
: m_sens(scalar), m_x(init_x), m_y(init_y)
{
    m_vga_buf[80*m_y+m_x] = ((m_vga_buf[80*m_y+m_x] & 0xF000) >> 4) 
                        | ((m_vga_buf[80*m_y+m_x] & 0x0F00) << 4) 
                        | (m_vga_buf[80*m_y+m_x] & 0x00FF); // Invert color
}

void TextualMouseHandler::on_move(int8_t x, int8_t y){
    m_vga_buf[80*m_y+m_x] = ((m_vga_buf[80*m_y+m_x] & 0xF000) >> 4) 
                        | ((m_vga_buf[80*m_y+m_x] & 0x0F00) << 4) 
                        | (m_vga_buf[80*m_y+m_x] & 0x00FF); // Invert color
    const uint8_t x_scalar = m_sens;
    const uint8_t y_scalar = 2*m_sens; // text mode cells are X by 2*X
    
    // Set x
    m_accum_x += x;
    // Set y
    m_accum_y += y;

    //handle bulk movements
    while(m_accum_x >= x_scalar){
        m_x++;
        m_accum_x -= x_scalar;
    }
    while(m_accum_x <= -x_scalar){
        m_x--;
        m_accum_x += x_scalar;
    }

    while(m_accum_y >= y_scalar){
        m_y++;
        m_accum_y -= y_scalar;
    }
    while(m_accum_y <= -y_scalar){
        m_y--;
        m_accum_y += y_scalar;
    }

    if( m_x < 0 )   m_x=0;
    if(m_x >= 80)   m_x=79;
    if( m_y < 0 )   m_y=0;
    if(m_y >= 25)   m_y=24;

    m_vga_buf[80*m_y+m_x] = ((m_vga_buf[80*m_y+m_x] & 0xF000) >> 4) 
                    | ((m_vga_buf[80*m_y+m_x] & 0x0F00) << 4) 
                    | (m_vga_buf[80*m_y+m_x] & 0x00FF); // Invert color

}

void TextualMouseHandler::on_up(uint8_t button){
    m_vga_buf[80*m_y+m_x] = ((m_vga_buf[80*m_y+m_x] & 0xF000) >> 4) 
                            | ((m_vga_buf[80*m_y+m_x] & 0x0F00) << 4) 
                            | (m_vga_buf[80*m_y+m_x] & 0x00FF);
}

void TextualMouseHandler::on_down(uint8_t button){
    m_vga_buf[80*m_y+m_x] = ((m_vga_buf[80*m_y+m_x] & 0xF000) >> 4) 
                            | ((m_vga_buf[80*m_y+m_x] & 0x0F00) << 4) 
                            | (m_vga_buf[80*m_y+m_x] & 0x00FF);
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
    if(m_handler == 0){
        return esp;
    }

    m_buf[m_offset] = m_data_port.read();
    m_offset = (m_offset + 1) % 3;
    
    if(m_offset==0){
        if(m_buf[1] != 0 || m_buf[2] != 0){
            m_handler->on_move(m_buf[1], -m_buf[2]);
        }
        for(uint8_t i=0; i<3; ++i){
            if((m_buf[0] & (0x1<<i)) != (m_buttons & (0x1<<i))){
                if(m_buttons & (0x1<<i)){
                    m_handler->on_up(i+1);
                }
                else{
                    m_handler->on_down(i+1);
                }
            }
        }
        m_buttons = m_buf[0];
    }

    return esp;
}