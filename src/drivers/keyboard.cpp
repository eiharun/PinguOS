#include <drivers/keyboard.h>
#include <hardware_communication/interrupts.h>

void printf(int8_t* string);

KeyboardDriver::KeyboardDriver(InterruptManager* interrupt_manager)
: InterruptHandler(KEYBOARD_INT, interrupt_manager),
data_port(0x60),
cmd_port(0x64){
    while(cmd_port.read() & 0x1){
        data_port.read();
    }
    cmd_port.write(0xAE); // activate interrupts
    cmd_port.write(0x20); // get current state
    uint8_t status = (data_port.read() | 1) & ~0x10;
    cmd_port.write(0x60); // set state
    data_port.write(status);

    data_port.write(0xF4);  

}

KeyboardDriver::~KeyboardDriver(){}

uint32_t KeyboardDriver::handle_interrupt(uint32_t esp) {
    uint16_t key = data_port.read();
    static bool shift_pressed = false;

    switch (key) {
        // ---------- Control keys ----------
        case KEY_ESC:        /* handle */ break;
        case KEY_BACKSPACE:  printf("\b"); break;
        case KEY_TAB:        printf("\t"); break;
        case KEY_ENTER:      printf("\n"); break;
        case KEY_SPACE:      printf(" "); break;

        // ---------- Number row ----------
        case KEY_1: shift_pressed ? printf("!") : printf("1"); break;
        case KEY_2: shift_pressed ? printf("@") : printf("2"); break;
        case KEY_3: shift_pressed ? printf("#") : printf("3"); break;
        case KEY_4: shift_pressed ? printf("$") : printf("4"); break;
        case KEY_5: shift_pressed ? printf("%") : printf("5"); break;
        case KEY_6: shift_pressed ? printf("^") : printf("6"); break;
        case KEY_7: shift_pressed ? printf("&") : printf("7"); break;
        case KEY_8: shift_pressed ? printf("*") : printf("8"); break;
        case KEY_9: shift_pressed ? printf("(") : printf("9"); break;
        case KEY_0: shift_pressed ? printf(")") : printf("0"); break;
        case KEY_MINUS:  shift_pressed ? printf("_") : printf("-"); break;
        case KEY_EQUAL:  shift_pressed ? printf("+") : printf("="); break;

        // ---------- Top row ----------
        case KEY_Q: shift_pressed ? printf("Q") : printf("q"); break;
        case KEY_W: shift_pressed ? printf("W") : printf("w"); break;
        case KEY_E: shift_pressed ? printf("E") : printf("e"); break;
        case KEY_R: shift_pressed ? printf("R") : printf("r"); break;
        case KEY_T: shift_pressed ? printf("T") : printf("t"); break;
        case KEY_Y: shift_pressed ? printf("Y") : printf("y"); break;
        case KEY_U: shift_pressed ? printf("U") : printf("u"); break;
        case KEY_I: shift_pressed ? printf("I") : printf("i"); break;
        case KEY_O: shift_pressed ? printf("O") : printf("o"); break;
        case KEY_P: shift_pressed ? printf("P") : printf("p"); break;
        case KEY_LBRACKET: shift_pressed ? printf("{") : printf("["); break;
        case KEY_RBRACKET: shift_pressed ? printf("}") : printf("]"); break;
        case KEY_BACKSLASH: shift_pressed ? printf("|") : printf("\\"); break;

        // ---------- Home row ----------
        case KEY_A: shift_pressed ? printf("A") : printf("a"); break;
        case KEY_S: shift_pressed ? printf("S") : printf("s"); break;
        case KEY_D: shift_pressed ? printf("D") : printf("d"); break;
        case KEY_F: shift_pressed ? printf("F") : printf("f"); break;
        case KEY_G: shift_pressed ? printf("G") : printf("g"); break;
        case KEY_H: shift_pressed ? printf("H") : printf("h"); break;
        case KEY_J: shift_pressed ? printf("J") : printf("j"); break;
        case KEY_K: shift_pressed ? printf("K") : printf("k"); break;
        case KEY_L: shift_pressed ? printf("L") : printf("l"); break;
        case KEY_SEMICOLON: shift_pressed ? printf(":") : printf(";"); break;
        case KEY_APOSTROPHE: shift_pressed ? printf("\"") : printf("'"); break;

        // ---------- Bottom row ----------
        case KEY_Z: shift_pressed ? printf("Z") : printf("z"); break;
        case KEY_X: shift_pressed ? printf("X") : printf("x"); break;
        case KEY_C: shift_pressed ? printf("C") : printf("c"); break;
        case KEY_V: shift_pressed ? printf("V") : printf("v"); break;
        case KEY_B: shift_pressed ? printf("B") : printf("b"); break;
        case KEY_N: shift_pressed ? printf("N") : printf("n"); break;
        case KEY_M: shift_pressed ? printf("M") : printf("m"); break;
        case KEY_COMMA: shift_pressed ? printf("<") : printf(","); break;
        case KEY_DOT: shift_pressed ? printf(">") : printf("."); break;
        case KEY_SLASH: shift_pressed ? printf("?") : printf("/"); break;

        // ---------- Modifiers ----------
        case KEY_LSHIFT: shift_pressed = true; break;
        case KEY_RSHIFT: shift_pressed = true; break;
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
        case 0xFA: break;
        case 0xC5: break;

        case KEY_LSHIFT_RELEASE: shift_pressed = false; break;
        case KEY_RSHIFT_RELEASE: shift_pressed = false; break;
        case KEY_LCTRL_RELEASE:  /* TODO: handle Ctrl release modifier */ break;
        case KEY_RCTRL_RELEASE:  /* TODO: handle Ctrl release modifier */ break;
        case KEY_LALT_RELEASE:   /* TODO: handle Alt release modifier */ break;
        case KEY_RALT_RELEASE:   /* TODO: handle Alt release modifier */ break;
        default:
            if (key<0x80){
                char* foo = " KEYBOARD 0x00";
                char* hex = "0123456789ABCDEF";
                foo[12] = hex[(key >> 4) & 0x0F];
                foo[13] = hex[key & 0x0F];
                printf(foo);
            }
    }

    return esp;
}