#include <hardware_communication/port.h>


Port::Port(uint16_t portnumber): m_portnumber(portnumber){};
Port::~Port(){}

Port8::Port8(uint16_t portnumber): Port(portnumber){}
Port8::~Port8(){}

void Port8::write(uint8_t data){
    __asm__ volatile("outb %0, %1" : : "a" (data), "Nd" (m_portnumber));
    // no output
    // inputs: data -> AL register, portnumber->imm or DX register
    // "outb AL, DX"
}

uint8_t Port8::read(){
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a" (result) : "Nd" (m_portnumber));
    // output: write only AL register -> result
    // input: portnumber -> imm or DX register
    // "inb DX, AL"
    return result;
}

Port8_Slow::Port8_Slow(uint16_t portnumber): Port8(portnumber){};
Port8_Slow::~Port8_Slow(){}

void Port8_Slow::write(uint8_t data){
    __asm__ volatile("outb %0, %1\njmp 1f\n1: jmp 1f\n1:" : : "a" (data), "Nd" (m_portnumber));
    //1f means jump to the next 1: label forwards (1b means backwards)

}

Port16::Port16(uint16_t portnumber): Port(portnumber){}
Port16::~Port16(){}

void Port16::write(uint16_t data){
    __asm__ volatile("outw %0, %1" : : "a" (data), "Nd" (m_portnumber));
}

uint16_t Port16::read(){
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a" (result) : "Nd" (m_portnumber));
    return result;
}

Port32::Port32(uint16_t portnumber): Port(portnumber){}
Port32::~Port32(){}

void Port32::write(uint32_t data){
    __asm__ volatile("outl %0, %1" : : "a" (data), "Nd" (m_portnumber));
}

uint32_t Port32::read(){
    uint32_t result;
    __asm__ volatile("inl %1, %0" : "=a" (result) : "Nd" (m_portnumber));
    return result;
}