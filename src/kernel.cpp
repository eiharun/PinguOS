#include <hardware_communication/interrupts.h>
#include <common/types.h>
#include <gdt.h>
#include <hardware_communication/port.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/driver.h>

using namespace drivers;

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
    DriverManager driver_manager;
    
    TextualKeyboardHandler keyboard_handle;
    // 80x25 mouse resolution -> map to 1920x1080 mouse resolution
    // 1.79999999856 *
    TextualMouseHandler mouse_handle(40,12,4);

    MouseDriver mouse(&interrupts, &mouse_handle);
    driver_manager.add_driver(&mouse);
    KeyboardDriver keyboard(&interrupts, &keyboard_handle);
    driver_manager.add_driver(&keyboard);

    driver_manager.activate_all();

    interrupts.activate();
    while(1);


}