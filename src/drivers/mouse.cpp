#include <drivers/mouse.h>


MouseDriver::MouseDriver(InterruptManager* interrupt_manager)
: InterruptHandler(MOUSE_INT, interrupt_manager),
m_data_port(0x60),
m_cmd_port(0x64)
{
     
}

void MouseDriver::activate(){
    m_offset = 0;
    m_buttons = 0;
    static uint16_t* vga_buf = (uint16_t*) 0xB8000;
    vga_buf[80*12+40] = ((vga_buf[80*12+40] & 0xF000) >> 4) | ((vga_buf[80*12+40] & 0x0F00) << 4) | (vga_buf[80*12+40] & 0x00FF);
        
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
    
    static int32_t x{40},y{12};

    m_buf[m_offset] = m_data_port.read();
    m_offset = (m_offset + 1) % 3;

    if(m_offset==0){
        static uint16_t* vga_buf = (uint16_t*) 0xB8000;
        vga_buf[80*y+x] = ((vga_buf[80*y+x] & 0xF000) >> 4) 
                        | ((vga_buf[80*y+x] & 0x0F00) << 4) 
                        | (vga_buf[80*y+x] & 0x00FF);
        x += m_buf[1]/6;
        if( x < 0 ){
            x=0;
        }
        if(x >= 80){
            x=79;
        }

        y -= m_buf[2]/6;
        if( y < 0 ){
            y=0;
        }
        if(y >= 25){
            y=24;
        }

        vga_buf[80*y+x] = ((vga_buf[80*y+x] & 0xF000) >> 4) 
                        | ((vga_buf[80*y+x] & 0x0F00) << 4) 
                        | (vga_buf[80*y+x] & 0x00FF);

        for(int i=0; i < 3; ++i){
            if( (m_buf[0] & (0x01 << i)) != (m_buttons & (0x01 << i))){
                vga_buf[80*y+x] = ((vga_buf[80*y+x] & 0xF000) >> 4) 
                        | ((vga_buf[80*y+x] & 0x0F00) << 4) 
                        | (vga_buf[80*y+x] & 0x00FF);
            }
        }
        m_buttons = m_buf[0];
    }

    return esp;
}