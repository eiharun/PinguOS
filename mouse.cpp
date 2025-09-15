#include "mouse.h"

MouseDriver::MouseDriver(InterruptManager* interrupt_manager)
: InterruptHandler(MOUSE_INT, interrupt_manager),
data_port(0x60),
cmd_port(0x64){
    offset = 0;
    buttons = 0;
    static uint16_t* vga_buf = (uint16_t*) 0xB8000;
    vga_buf[80*12+40] = ((vga_buf[80*12+40] & 0xF000) >> 4) | ((vga_buf[80*12+40] & 0x0F00) << 4) | (vga_buf[80*12+40] & 0x00FF);
        
    cmd_port.write(0xA8); // activate interrupts
    cmd_port.write(0x20); // get current state
    uint8_t status = data_port.read() | 2;
    cmd_port.write(0x60); // set state
    data_port.write(status);
    cmd_port.write(0xD4);
    data_port.write(0xF4);  
    data_port.read();  

}

MouseDriver::~MouseDriver(){}

uint32_t MouseDriver::handle_interrupt(uint32_t esp){
    uint8_t status = cmd_port.read();
    if(!(status & 0x20)){
        return esp;
    }
    
    static int32_t x{40},y{12};

    buf[offset] = data_port.read();
    offset = (offset + 1) % 3;

    if(offset==0){
        static uint16_t* vga_buf = (uint16_t*) 0xB8000;
        vga_buf[80*y+x] = ((vga_buf[80*y+x] & 0xF000) >> 4) 
                        | ((vga_buf[80*y+x] & 0x0F00) << 4) 
                        | (vga_buf[80*y+x] & 0x00FF);
        x += buf[1];
        if( x < 0 ){
            x=0;
        }
        if(x >= 80){
            x=79;
        }
        y -= buf[2];
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
            if( (buf[0] & (0x01 << i)) != (buttons & (0x01 << i))){
                vga_buf[80*y+x] = ((vga_buf[80*y+x] & 0xF000) >> 4) 
                        | ((vga_buf[80*y+x] & 0x0F00) << 4) 
                        | (vga_buf[80*y+x] & 0x00FF);
            }
        }
        buttons = buf[0];
    }

    return esp;
}