#include <hardware_communication/interrupts.h>
#include <common/types.h>
#include <gdt.h>
#include <hardware_communication/port.h>
#include <hardware_communication/pci.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <drivers/driver.h>
#include <gui/desktop.h>
#include <gui/window.h>

using namespace drivers;
using namespace gui;

void printf(int8_t* string){
    static uint16_t* vga_buffer = (uint16_t*) 0xb8000;
    static int x{}, y{}; //VGA text mode is 80 by 25 char
    const int WIDTH{80}, HEIGHT{25};
    for(int i=0; string[i] != '\0'; ++i){
        switch (string[i]){
        case '\n':
            x=0;
            y++;
            if(y>=HEIGHT){
                for(int i=0; i<WIDTH*HEIGHT; ++i){
                    vga_buffer[i] = (vga_buffer[i] & 0xFF00) | ' ';
                }
                x=0;
                y=0;
            }
            break;
        default:
            vga_buffer[(WIDTH*y)+x] = (vga_buffer[(WIDTH*y)+x] & 0xFF00) | string[i];
            // (80 * row) + col; mult 80 by the row creates an offset for the rows in memory
            x++;
            if(x>=WIDTH){
                x=0;
                y++;
            }

            if(y>=HEIGHT){
                for(int i=0; i<WIDTH*HEIGHT; ++i){
                    vga_buffer[i] = (vga_buffer[i] & 0xFF00) | ' ';
                }
                x=0;
                y=0;
            }
            break;
        }

    }
}

void printf_hex(uint8_t value){
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(value >> 4) & 0x0F];
    foo[1] = hex[value & 0x0F];
    printf(foo);
}

typedef void (*constructor)(); //function pointer
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void call_constructors(){
    for(constructor* i=&start_ctors; i!=&end_ctors; ++i){
        (*i)();
    }
}

extern "C" void pingu_kernel_main(void* multiboot_struct, uint32_t magic_number){
    printf("Noot Noot!         ");
    printf("Piingu Piinguu! \n");
    
    GlobalDescriptorTable gdt;
    InterruptManager interrupts(&gdt);
    
    rgb color{0x00,0x00,0xA8};
    Desktop desktop(320,200, color);
    
    DriverManager driver_manager;
    
    printf("Initializing Hardware\n");

    // TextualKeyboardHandler keyboard_handle;
    // TextualMouseHandler mouse_handle(40,12,9);
    
    MouseDriver mouse(&interrupts, &desktop);
    driver_manager.add_driver(&mouse);
    KeyboardDriver keyboard(&interrupts, &desktop);
    driver_manager.add_driver(&keyboard);
    
    PCIController pci_controller;
    pci_controller.select_drivers(&driver_manager, &interrupts);
    
    VideoGraphicsArray vga;
    
    printf("Activating Drivers\n");
    driver_manager.activate_all();
    printf("Activating Interrupts\n");
    for(int j=0; j<1000000000; j++){}
    
    Window window1(&desktop, 10, 10, 20, 20, rgb{0xA8,0x00,0x00});
    desktop.add_child(&window1);
    Window window2(&desktop, 40, 15, 25, 25, rgb{0x00,0xA8,0x00});
    desktop.add_child(&window2);

    vga.set_mode(320, 200, 8);
    interrupts.activate();
    while(1){
        desktop.draw(&vga);
    }
}