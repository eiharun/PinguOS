#include <drivers/keyboard.h>
#include <hardware_communication/interrupts.h>

void printf(int8_t* string);

KeyboardHandler::KeyboardHandler(){}

TextualKeyboardHandler::TextualKeyboardHandler()
:m_shift(false)
{}

void TextualKeyboardHandler::emit_event(uint16_t key){
    if(key < 0x80){
        switch(key){
        // ---------- Control keys ----------
        case KEY_ESC:        /* handle */ break;
        case KEY_BACKSPACE:  printf("\b"); break;
        case KEY_TAB:        printf("\t"); break;
        case KEY_ENTER:      printf("\n"); break;
        case KEY_SPACE:      printf(" "); break;

        // ---------- Number row ----------
        case KEY_1: m_shift ? printf("!") : printf("1"); break;
        case KEY_2: m_shift ? printf("@") : printf("2"); break;
        case KEY_3: m_shift ? printf("#") : printf("3"); break;
        case KEY_4: m_shift ? printf("$") : printf("4"); break;
        case KEY_5: m_shift ? printf("%") : printf("5"); break;
        case KEY_6: m_shift ? printf("^") : printf("6"); break;
        case KEY_7: m_shift ? printf("&") : printf("7"); break;
        case KEY_8: m_shift ? printf("*") : printf("8"); break;
        case KEY_9: m_shift ? printf("(") : printf("9"); break;
        case KEY_0: m_shift ? printf(")") : printf("0"); break;
        case KEY_MINUS:  m_shift ? printf("_") : printf("-"); break;
        case KEY_EQUAL:  m_shift ? printf("+") : printf("="); break;

        // ---------- Top row ----------
        case KEY_Q: m_shift ? printf("Q") : printf("q"); break;
        case KEY_W: m_shift ? printf("W") : printf("w"); break;
        case KEY_E: m_shift ? printf("E") : printf("e"); break;
        case KEY_R: m_shift ? printf("R") : printf("r"); break;
        case KEY_T: m_shift ? printf("T") : printf("t"); break;
        case KEY_Y: m_shift ? printf("Y") : printf("y"); break;
        case KEY_U: m_shift ? printf("U") : printf("u"); break;
        case KEY_I: m_shift ? printf("I") : printf("i"); break;
        case KEY_O: m_shift ? printf("O") : printf("o"); break;
        case KEY_P: m_shift ? printf("P") : printf("p"); break;
        case KEY_LBRACKET: m_shift ? printf("{") : printf("["); break;
        case KEY_RBRACKET: m_shift ? printf("}") : printf("]"); break;
        case KEY_BACKSLASH: m_shift ? printf("|") : printf("\\"); break;

        // ---------- Home row ----------
        case KEY_A: m_shift ? printf("A") : printf("a"); break;
        case KEY_S: m_shift ? printf("S") : printf("s"); break;
        case KEY_D: m_shift ? printf("D") : printf("d"); break;
        case KEY_F: m_shift ? printf("F") : printf("f"); break;
        case KEY_G: m_shift ? printf("G") : printf("g"); break;
        case KEY_H: m_shift ? printf("H") : printf("h"); break;
        case KEY_J: m_shift ? printf("J") : printf("j"); break;
        case KEY_K: m_shift ? printf("K") : printf("k"); break;
        case KEY_L: m_shift ? printf("L") : printf("l"); break;
        case KEY_SEMICOLON: m_shift ? printf(":") : printf(";"); break;
        case KEY_APOSTROPHE: m_shift ? printf("\"") : printf("'"); break;

        // ---------- Bottom row ----------
        case KEY_Z: m_shift ? printf("Z") : printf("z"); break;
        case KEY_X: m_shift ? printf("X") : printf("x"); break;
        case KEY_C: m_shift ? printf("C") : printf("c"); break;
        case KEY_V: m_shift ? printf("V") : printf("v"); break;
        case KEY_B: m_shift ? printf("B") : printf("b"); break;
        case KEY_N: m_shift ? printf("N") : printf("n"); break;
        case KEY_M: m_shift ? printf("M") : printf("m"); break;
        case KEY_COMMA: m_shift ? printf("<") : printf(","); break;
        case KEY_DOT: m_shift ? printf(">") : printf("."); break;
        case KEY_SLASH: m_shift ? printf("?") : printf("/"); break;

        // ---------- Modifiers ----------
        case KEY_LSHIFT: m_shift = true; break;
        case KEY_RSHIFT: m_shift = true; break;
        case KEY_LCTRL:  /* TODO: handle Ctrl modifier */ break;
        case KEY_RCTRL:  /* TODO: handle Ctrl modifier */ break;
        case KEY_LALT:   /* TODO: handle Alt modifier */ break;
        case KEY_RALT:   /* TODO: handle Alt modifier */ break;
        case KEY_CAPSLOCK: /* TODO: toggle caps lock */ break;
        case KEY_NUMLOCK:  /* TODO: toggle num lock */ break;
        case KEY_SCROLLLOCK: /* TODO: toggle scroll lock */ break;

        // ---------- Function keys ----------
        case KEY_F1:  /* TODO */ break;
        case KEY_F2:  /* TODO */ break;
        case KEY_F3:  /* TODO */ break;
        case KEY_F4:  /* TODO */ break;
        case KEY_F5:  /* TODO */ break;
        case KEY_F6:  /* TODO */ break;
        case KEY_F7:  /* TODO */ break;
        case KEY_F8:  /* TODO */ break;
        case KEY_F9:  /* TODO */ break;
        case KEY_F10: /* TODO */ break;
        case KEY_F11: /* TODO */ break;
        case KEY_F12: /* TODO */ break;

        // ---------- Keypad ----------
        case KEY_KP0: printf("0"); break;
        case KEY_KP1: printf("1"); break;
        case KEY_KP2: printf("2"); break;
        case KEY_KP3: printf("3"); break;
        case KEY_KP4: printf("4"); break;
        case KEY_KP5: printf("5"); break;
        case KEY_KP6: printf("6"); break;
        case KEY_KP7: printf("7"); break;
        case KEY_KP8: printf("8"); break;
        case KEY_KP9: printf("9"); break;
        case KEY_KP_DOT: printf("."); break;
        case KEY_KP_PLUS: printf("+"); break;
        case KEY_KP_MINUS: printf("-"); break;
        case KEY_KP_ASTERISK: printf("*"); break;
        case KEY_KP_SLASH: printf("/"); break;
        case KEY_KP_ENTER: printf("\n"); break;

        // ---------- Navigation (extended codes) ----------
        case KEY_UP:    /* TODO: handle cursor up */ break;
        case KEY_DOWN:  /* TODO: handle cursor down */ break;
        case KEY_LEFT:  /* TODO: handle cursor left */ break;
        case KEY_RIGHT: /* TODO: handle cursor right */ break;
        case KEY_HOME:  /* TODO: handle home */ break;
        case KEY_END:   /* TODO: handle end */ break;
        case KEY_PGUP:  /* TODO: handle page up */ break;
        case KEY_PGDN:  /* TODO: handle page down */ break;
        case KEY_INSERT:/* TODO: handle insert */ break;
        case KEY_DELETE:/* TODO: handle delete */ break;

        // ---------- OS keys ----------
        case KEY_LWIN:  /* TODO: handle left Windows key */ break;
        case KEY_RWIN:  /* TODO: handle right Windows key */ break;
        case KEY_MENU:  /* TODO: handle menu key */ break;

        // ---------- Special ----------
        // Print Screen = multi-byte sequence (not a single scancode)
        // Pause/Break   = multi-byte sequence (not a single scancode)
        // -----------------------------
        case 0xFA: break;
        case 0xC5: break;

        case KEY_LSHIFT_RELEASE: m_shift = false; break;
        case KEY_RSHIFT_RELEASE: m_shift = false; break;
        case KEY_LCTRL_RELEASE:  /* TODO: handle Ctrl release modifier */ break;
        case KEY_RCTRL_RELEASE:  /* TODO: handle Ctrl release modifier */ break;
        case KEY_LALT_RELEASE:   /* TODO: handle Alt release modifier */ break;
        case KEY_RALT_RELEASE:   /* TODO: handle Alt release modifier */ break;
        default:
            char* foo = " KEYBOARD 0x00";
            char* hex = "0123456789ABCDEF";
            foo[12] = hex[(key >> 4) & 0x0F];
            foo[13] = hex[key & 0x0F];
            printf(foo);
        }
    }
    else{
        switch(key | 0x80){
        case KEY_LSHIFT_RELEASE: m_shift = false; break;
        case KEY_RSHIFT_RELEASE: m_shift = false; break;
        case KEY_LCTRL_RELEASE:  /* TODO: handle Ctrl release modifier */ break;
        case KEY_RCTRL_RELEASE:  /* TODO: handle Ctrl release modifier */ break;
        case KEY_LALT_RELEASE:   /* TODO: handle Alt release modifier */ break;
        case KEY_RALT_RELEASE:   /* TODO: handle Alt release modifier */ break;
        }
    }
}

void TextualKeyboardHandler::on_key_up(char key){

}

void TextualKeyboardHandler::on_key_down(char key){
    char* temp = " ";
    if(m_shift == true){
        if(key >= 'a' && key >= 'z'){
            key = key - 'a' + 'A';
        }
        else{
            switch(key){
            case '`': key = '~'; break;
            case '1': key = '!'; break;
            }
        }
    }
    else{
        temp[0] = key;
    }
    printf(temp);
}

void TextualKeyboardHandler::set_shift(bool state){
    m_shift = state;
}

KeyboardDriver::KeyboardDriver(InterruptManager* interrupt_manager, KeyboardHandler* event_handler)
: InterruptHandler(KEYBOARD_INT, interrupt_manager),
m_data_port(0x60),
m_cmd_port(0x64),
m_event_handler(event_handler)
{

}

void KeyboardDriver::activate(){
    while(m_cmd_port.read() & 0x1){
        m_data_port.read();
    }
    m_cmd_port.write(0xAE); // activate interrupts
    m_cmd_port.write(0x20); // get current state
    uint8_t status = (m_data_port.read() | 1) & ~0x10;
    m_cmd_port.write(0x60); // set state
    m_data_port.write(status);

    m_data_port.write(0xF4);  
}

KeyboardDriver::~KeyboardDriver(){}

uint32_t KeyboardDriver::handle_interrupt(uint32_t esp) {
    uint16_t key = m_data_port.read();
    static bool m_shift = false;
    m_event_handler->emit_event(key);

    return esp;
}