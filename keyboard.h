#pragma once
#include "types.h"
#include "interrupts.h"
#include "port.h"

class KeyboardDriver: public InterruptHandler{    
public:
    KeyboardDriver(InterruptManager* interrupt_manger);
    ~KeyboardDriver();
    virtual uint32_t handle_interrupt(uint32_t esp) override;
    
private:
    Port8 data_port;
    Port8 cmd_port;
    
};

#define KEYBOARD_INT 0x21

// ---------------- Normal Scan Codes ----------------
#define KEY_ESC          0x01
#define KEY_1            0x02
#define KEY_2            0x03
#define KEY_3            0x04
#define KEY_4            0x05
#define KEY_5            0x06
#define KEY_6            0x07
#define KEY_7            0x08
#define KEY_8            0x09
#define KEY_9            0x0A
#define KEY_0            0x0B
#define KEY_MINUS        0x0C
#define KEY_EQUAL        0x0D
#define KEY_BACKSPACE    0x0E
#define KEY_TAB          0x0F
#define KEY_Q            0x10
#define KEY_W            0x11
#define KEY_E            0x12
#define KEY_R            0x13
#define KEY_T            0x14
#define KEY_Y            0x15
#define KEY_U            0x16
#define KEY_I            0x17
#define KEY_O            0x18
#define KEY_P            0x19
#define KEY_LBRACKET     0x1A
#define KEY_RBRACKET     0x1B
#define KEY_ENTER        0x1C
#define KEY_LCTRL        0x1D
#define KEY_A            0x1E
#define KEY_S            0x1F
#define KEY_D            0x20
#define KEY_F            0x21
#define KEY_G            0x22
#define KEY_H            0x23
#define KEY_J            0x24
#define KEY_K            0x25
#define KEY_L            0x26
#define KEY_SEMICOLON    0x27
#define KEY_APOSTROPHE   0x28
#define KEY_GRAVE        0x29  // `
#define KEY_LSHIFT       0x2A
#define KEY_BACKSLASH    0x2B
#define KEY_Z            0x2C
#define KEY_X            0x2D
#define KEY_C            0x2E
#define KEY_V            0x2F
#define KEY_B            0x30
#define KEY_N            0x31
#define KEY_M            0x32
#define KEY_COMMA        0x33
#define KEY_DOT          0x34
#define KEY_SLASH        0x35
#define KEY_RSHIFT       0x36
#define KEY_KP_ASTERISK  0x37
#define KEY_LALT         0x38
#define KEY_SPACE        0x39
#define KEY_CAPSLOCK     0x3A
#define KEY_F1           0x3B
#define KEY_F2           0x3C
#define KEY_F3           0x3D
#define KEY_F4           0x3E
#define KEY_F5           0x3F
#define KEY_F6           0x40
#define KEY_F7           0x41
#define KEY_F8           0x42
#define KEY_F9           0x43
#define KEY_F10          0x44
#define KEY_NUMLOCK      0x45
#define KEY_SCROLLLOCK   0x46
#define KEY_KP7          0x47
#define KEY_KP8          0x48
#define KEY_KP9          0x49
#define KEY_KP_MINUS     0x4A
#define KEY_KP4          0x4B
#define KEY_KP5          0x4C
#define KEY_KP6          0x4D
#define KEY_KP_PLUS      0x4E
#define KEY_KP1          0x4F
#define KEY_KP2          0x50
#define KEY_KP3          0x51
#define KEY_KP0          0x52
#define KEY_KP_DOT       0x53
#define KEY_SYSREQ       0x54  // rarely used
#define KEY_F11          0x57
#define KEY_F12          0x58

// ---------------- Extended (0xE0 prefix) ----------------
#define KEY_KP_ENTER     0xE01C
#define KEY_RCTRL        0xE01D
#define KEY_KP_SLASH     0xE035
#define KEY_RALT         0xE038
#define KEY_HOME         0xE047
#define KEY_UP           0xE048
#define KEY_PGUP         0xE049
#define KEY_LEFT         0xE04B
#define KEY_RIGHT        0xE04D
#define KEY_END          0xE04F
#define KEY_DOWN         0xE050
#define KEY_PGDN         0xE051
#define KEY_INSERT       0xE052
#define KEY_DELETE       0xE053
#define KEY_LWIN         0xE05B
#define KEY_RWIN         0xE05C
#define KEY_MENU         0xE05D

// ---------------- Special Multi-Byte Keys ----------------
// Print Screen: E0 2A E0 37 (press), E0 B7 E0 AA (release)
// Pause/Break:  E1 1D 45 (press), E1 9D C5 (release)


// ------------------- Key Releases ------------------------
#define KEY_LSHIFT_RELEASE  (KEY_LSHIFT | 0x80)
#define KEY_RSHIFT_RELEASE  (KEY_RSHIFT | 0x80)
#define KEY_LCTRL_RELEASE   (KEY_LCTRL  | 0x80)
#define KEY_RCTRL_RELEASE   (KEY_RCTRL  | 0x80)
#define KEY_LALT_RELEASE    (KEY_LALT   | 0x80)
#define KEY_RALT_RELEASE    (KEY_RALT   | 0x80)