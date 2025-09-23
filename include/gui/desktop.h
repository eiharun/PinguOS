#pragma once
#include <common/graphics_context.h>
#include <drivers/mouse.h>
#include <gui/widget.h>
#include <common/types.h>

using namespace common;

namespace gui {

class Desktop: public CompositeWidget, public drivers::MouseHandler{
protected:
    uint32_t m_x;
    uint32_t m_y;
public:
    Desktop(uint32_t w, uint32_t h, rgb color);
    ~Desktop();

    void draw(GraphicsContext* gc) override;

    void on_up(uint8_t button) override;
    void on_down(uint8_t button) override;
    void on_move(int8_t x, int8_t y) override;
};


}