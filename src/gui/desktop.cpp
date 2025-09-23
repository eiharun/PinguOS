#include "drivers/mouse.h"
#include "gui/widget.h"
#include <gui/desktop.h>

using namespace gui;

Desktop::Desktop(uint32_t w, uint32_t h, rgb color)
: CompositeWidget(0,0,0, w, h, color), drivers::MouseHandler(){
    m_x = w/2;
    m_y = h/2;
}
Desktop::~Desktop(){}

void Desktop::draw(GraphicsContext* gc){
    CompositeWidget::draw(gc);  
    // Cursor
    for(int i=0; i<4; i++){
        gc->put_pixel(m_x-i, m_y, rgb{0xFF,0xFF,0xFF});
        gc->put_pixel(m_x+i, m_y, rgb{0xFF,0xFF,0xFF});
        gc->put_pixel(m_x, m_y-i, rgb{0xFF,0xFF,0xFF});
        gc->put_pixel(m_x, m_y+i, rgb{0xFF,0xFF,0xFF});
    }
}

void Desktop::on_up(uint8_t button){
    CompositeWidget::on_mouse_up(m_x, m_y, button);
}

void Desktop::on_down(uint8_t button){
    CompositeWidget::on_mouse_down(m_x, m_y, button);
}

void Desktop::on_move(int8_t x, int8_t y){
    // x /= 2;
    // y /= 2; // TODO: Implement accumulation method before scaling down
    int32_t new_x = m_x + x;
    if(new_x < 0) new_x = 0;
    if(new_x > m_w) new_x = m_w-1;

    int32_t new_y = m_y + y;
    if(new_y < 0) new_y = 0;
    if(new_y > m_h) new_y = m_h-1;

    CompositeWidget::on_mouse_move(m_x, m_y, new_x, new_y);

    m_x = new_x;
    m_y = new_y;
}
