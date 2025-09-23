#include "gui/widget.h"
#include <gui/window.h>

using namespace gui;

Window::Window(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, rgb color)
: CompositeWidget(parent, x, y, w, h, color), m_dragged(false){

}
Window::~Window(){}

void Window::on_mouse_down(int32_t x, int32_t y, uint8_t button){
    if(button == 1) m_dragged = true;
    CompositeWidget::on_mouse_down(x, y, button);
}

void Window::on_mouse_up(int32_t x, int32_t y, uint8_t button){
    m_dragged = false;
    CompositeWidget::on_mouse_up(x, y, button);
}

void Window::on_mouse_move(int32_t old_x, int32_t old_y, int32_t new_x, int32_t new_y){
    if(m_dragged){
        m_x += new_x-old_x; 
        m_y += new_y-old_y; 
    }
    CompositeWidget::on_mouse_move(old_x, old_y, new_x, new_y);
}

