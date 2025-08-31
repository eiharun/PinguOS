
void printf(char* string){
    unsigned short* vga_buffer = (unsigned short*) 0xb8000;
    for(int i=0; string[i] != '\0'; ++i){
        vga_buffer[i] = (vga_buffer[i] & 0xFF00) | string[i];
    }
}

extern "C" void pingu_kernel_main(void* multiboot_struct, unsigned int magic_number){
    printf("Noot Noot!");

    while(1);

}