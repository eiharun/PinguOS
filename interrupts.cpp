#include "interrupts.h"

void printf(int8_t* string);

uint32_t InterruptManager::handleInterrupt(uint8_t interrupt_number, uint32_t esp){
    printf("  INTERRUPT");
    return esp;
}
