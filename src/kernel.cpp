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
#include <multitask.h>
#include <memory_management.h>
#include <common/macro.h>
#include <drivers/intel_82540em.h>
#include <drivers/ata.h>

// #define GRAPHICS_MODE

using namespace drivers;
using namespace gui;
using namespace multitasking;

void printf(int8_t* string);
void taskA(){
    while(true){
        printf("A");
    }
}
void taskB(){
    while(true){
        printf("B");
    }
}


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

void printf_hex16(uint16_t value){
    printf_hex((value >> 8) & 0xFF);
    printf_hex( value & 0xFF);
}

void printf_hex32(uint32_t value){
    printf_hex((value >> 24) & 0xFF);
    printf_hex((value >> 16) & 0xFF);
    printf_hex((value >> 8) & 0xFF);
    printf_hex( value & 0xFF);
}

typedef void (*constructor)(); //function pointer
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void call_constructors(){
    for(constructor* i=&start_ctors; i!=&end_ctors; ++i){
        (*i)();
    }
}

extern "C" void pingu_kernel_main(const void* multiboot_struct, uint32_t magic_number){
    printf("Noot Noot!         ");
    printf("Piingu Piinguu! \n");
    
    GlobalDescriptorTable gdt;

    uint32_t* mem_upper = (uint32_t*)(((size_t)multiboot_struct) + 8);
    size_t heap = 10*1024*1024; // 10MB
    memory_management::MemoryManager memory_manager(heap, (*mem_upper)*1024 - heap - 10*1024);

    printf("heap: 0x");
    printf_hex((heap >> 24) & 0xFF);
    printf_hex((heap >> 16) & 0xFF);
    printf_hex((heap >> 8) & 0xFF);
    printf_hex((heap) & 0xFF);

    uint32_t* allocated = (uint32_t*)memory_manager.malloc(1024);
    printf("\nallocated: 0x");
    printf_hex(((uint32_t)allocated >> 24) & 0xFF);
    printf_hex(((uint32_t)allocated >> 16) & 0xFF);
    printf_hex(((uint32_t)allocated >> 8) & 0xFF);
    printf_hex(((uint32_t)allocated) & 0xFF);
    printf("\n");

    TaskManager task_manager;
    // Task task1(&gdt, taskA);
    // Task task2(&gdt, taskB);
    // task_manager.add_task(&task1);
    // task_manager.add_task(&task2);

    InterruptManager interrupts(&gdt, &task_manager);
    
    #ifdef GRAPHICS_MODE
        rgb color{0x00,0x00,0xA8};
        Desktop desktop(320,200, color);
    #endif

    DriverManager driver_manager;
    
    printf("Initializing Hardware\n");

    #ifndef GRAPHICS_MODE
        TextualKeyboardHandler keyboard_handle;
        TextualMouseHandler mouse_handle(40,12,9);
    #endif

    #ifdef GRAPHICS_MODE
        MouseDriver mouse(&interrupts, &desktop);
        KeyboardDriver keyboard(&interrupts, &desktop);
    #else
        MouseDriver mouse(&interrupts, &mouse_handle);
        KeyboardDriver keyboard(&interrupts, &keyboard_handle);
    #endif

    driver_manager.add_driver(&mouse);
    driver_manager.add_driver(&keyboard);
    
    PCIController pci_controller;
    pci_controller.select_drivers(&driver_manager, &interrupts);
    
    VideoGraphicsArray vga;
    
    printf("Activating Drivers\n");
    driver_manager.activate_all();
    printf("Activating Interrupts\n");

    #ifdef GRAPHICS_MODE
    Window window1(&desktop, 10, 10, 20, 20, rgb{0xA8,0x00,0x00});
    desktop.add_child(&window1);
    Window window2(&desktop, 40, 15, 25, 25, rgb{0x00,0xA8,0x00});
    desktop.add_child(&window2);

    vga.set_mode(320, 200, 8);
    #endif
    // int 14
    ATA ata0m(0x1F0, true);
    printf("ATA Primary Master: ");
    ata0m.identify();
    ATA ata0s(0x1F0, false);
    printf("ATA Primary Slave: ");
    ata0s.identify();
    char* buffer = "PinguOnA Hard Disk Drive!";
    ata0m.write_28(0, (uint8_t*)buffer, 25);
    ata0m.flush();
    buffer = "\n";
    ata0m.read_28(0,(uint8_t*)buffer, 25);

    // int 15
    ATA ata1m(0x170, true);
    ATA ata1s(0x170, false);

    interrupts.activate();
    for(int i = 0; i<100000000; ++i){}
    // uint8_t data[5] = {'A', 'B', 'C', 'D', 'E'};
    // Intel_82540EM* eth0 = (Intel_82540EM*)(driver_manager.m_drivers[2]);
    // eth0->send_packet(data, 5);
    while(1){
        #ifdef GRAPHICS_MODE
            desktop.draw(&vga);
        #endif
    }
}