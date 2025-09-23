#include "drivers/keyboard.h"
#include <gui/widget.h>

using namespace gui;

Widget::Widget(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, rgb color):
m_parent(parent), m_x(x), m_y(y), m_w(w), m_h(h), m_color(color), m_focusable(true)
{

}

Widget::~Widget(){}

void Widget::get_focus(Widget* widget){
    if(m_parent != 0){
        m_parent->get_focus(widget);
    }
}

void Widget::model_to_screen(int32_t &x, int32_t &y){
    if(m_parent != 0){
        m_parent->model_to_screen(x, y);
    }
    x += m_x;
    y += m_y;
}

void Widget::draw(GraphicsContext* gc){
    int x = 0;
    int y = 0;
    model_to_screen(x, y);
    gc->fill_rectangle(x,y, m_w, m_h, m_color);
}

void Widget::on_mouse_down(int32_t x, int32_t y, uint8_t button){
    if(m_focusable){
        get_focus(this);
    }
}

void Widget::on_mouse_up(int32_t x, int32_t y, uint8_t button){

}

void Widget::on_mouse_move(int32_t old_x, int32_t old_y, int32_t new_x, int32_t new_y){

}

bool Widget::contains_coordinate(int32_t x, int32_t y){
    return m_x <= x && x < m_x + m_w && m_y <= y && y < m_y + m_h;
}



CompositeWidget::CompositeWidget(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, rgb color)
: Widget(parent, x, y, w, h, color){
    m_focused_child = 0;
    m_num_children = 0;
}
CompositeWidget::~CompositeWidget(){

}

void CompositeWidget::get_focus(Widget* widget){
    m_focused_child = widget;
    if(m_parent != 0){
        m_parent->get_focus(widget);
    }
}

bool CompositeWidget::add_child(Widget* child){
    if(m_num_children >= 100){
        return false;
    }
    m_children[m_num_children++] = child;
    return true;
}

void CompositeWidget::draw(GraphicsContext* gc){
    Widget::draw(gc);
    for(int i = m_num_children-1; i >= 0; --i){
        m_children[i]->draw(gc);
    }
}
void CompositeWidget::on_mouse_down(int32_t x, int32_t y, uint8_t button){
    for(int i = 0; i < m_num_children; ++i){
        if(m_children[i]->contains_coordinate(x-m_x, y-m_y)){
            m_children[i]->on_mouse_down(x-m_x, y-m_y, button);
            break;
        }
    }
}
void CompositeWidget::on_mouse_up(int32_t x, int32_t y, uint8_t button){
    for(int i = 0; i < m_num_children; ++i){
        if(m_children[i]->contains_coordinate(x-m_x, y-m_y)){
            m_children[i]->on_mouse_up(x-m_x, y-m_y, button);
            break;
        }
    }
}
void CompositeWidget::on_mouse_move(int32_t old_x, int32_t old_y, int32_t new_x, int32_t new_y){
    int first_child = -1;
    for(int i = 0; i < m_num_children; ++i){
        if(m_children[i]->contains_coordinate(old_x-m_x, old_y-m_y)){
            m_children[i]->on_mouse_move(old_x-m_x, old_y-m_y, new_x-m_x, new_y-m_y);
            first_child = i;
            break;
        }
    }

    for(int i = 0; i < m_num_children; ++i){
        if(m_children[i]->contains_coordinate(new_x-m_x, new_y-m_y)){
            if(first_child != i){
                m_children[i]->on_mouse_move(old_x-m_x, old_y-m_y, new_x-m_x, new_y-m_y);
            }
            break;
        }
    }
}

void CompositeWidget::on_key_down(char key){
    if(m_focused_child != 0){
        m_focused_child->on_key_down(key);
    }
}

void CompositeWidget::on_key_up(char key){

}

