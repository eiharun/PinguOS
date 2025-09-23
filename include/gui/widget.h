#pragma once
#include <common/types.h>
#include <common/graphics_context.h>
#include <drivers/keyboard.h>

using namespace common;

namespace gui {

class Widget: public drivers::KeyboardHandler{
protected:
    Widget* m_parent;
    int32_t m_x;
    int32_t m_y;
    int32_t m_w;
    int32_t m_h;

    rgb m_color;
    // uint8_t m_r;
    // uint8_t m_g;
    // uint8_t m_b; // todo use color struct?
    bool m_focusable;
public:
    Widget(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, rgb color);
    ~Widget();

    virtual void get_focus(Widget* widget);
    virtual void model_to_screen(int32_t &x, int32_t &y);

    virtual void draw(GraphicsContext* gc);
    virtual void on_mouse_down(int32_t x, int32_t y, uint8_t button);
    virtual void on_mouse_up(int32_t x, int32_t y, uint8_t button);
    virtual void on_mouse_move(int32_t old_x, int32_t old_y, int32_t new_x, int32_t new_y);

    virtual bool contains_coordinate(int32_t x, int32_t y);

};

class CompositeWidget: public Widget{
private:
    Widget* m_children[100];
    int m_num_children;
    Widget* m_focused_child;
public:
    CompositeWidget(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, rgb color);
    ~CompositeWidget();

    virtual void get_focus(Widget* widget) override;
    virtual bool add_child(Widget* child);

    virtual void draw(GraphicsContext* gc) override;
    virtual void on_mouse_down(int32_t x, int32_t y, uint8_t button) override;
    virtual void on_mouse_up(int32_t x, int32_t y, uint8_t button) override;
    virtual void on_mouse_move(int32_t old_x, int32_t old_y, int32_t new_x, int32_t new_y) override;

    virtual void on_key_up(char key) override;
    virtual void on_key_down(char key) override;
};

}