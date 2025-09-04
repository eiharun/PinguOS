#include "types.h"
#include "gdt.h"

void printf(int8_t* string){
    uint16_t* vga_buffer = (uint16_t*) 0xb8000;
    for(int i=0; string[i] != '\0'; ++i){
        vga_buffer[i] = (vga_buffer[i] & 0xFF00) | string[i];
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
    GlobalDescriptorTable gdt;

    while(1);

}