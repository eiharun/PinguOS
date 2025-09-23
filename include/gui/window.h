#pragma once
#include "gui/widget.h"
#include <common/types.h>

using namespace common;

namespace gui {

class Window: public CompositeWidget{
protected:
    bool m_dragged;
public:
    Window(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, rgb color);
    ~Window();

    void on_mouse_down(int32_t x, int32_t y, uint8_t button) override;
    void on_mouse_up(int32_t x, int32_t y, uint8_t button) override;
    void on_mouse_move(int32_t old_x, int32_t old_y, int32_t new_x, int32_t new_y) override;

};

}