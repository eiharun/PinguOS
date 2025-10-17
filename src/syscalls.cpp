#include <syscalls.h>

SyscallHandler::SyscallHandler(hardware_communication::InterruptManager* interrupt_manager, uint8_t interrupt_number)
: hardware_communication::InterruptHandler(interrupt_number + INTERRUPTS_BASE, interrupt_manager)
{

}

SyscallHandler::~SyscallHandler(){}

void printf(char*);
uint32_t SyscallHandler::handle_interrupt(uint32_t esp){
    multitasking::CPUState* cpu = (multitasking::CPUState*)esp;
    switch(cpu->eax){
    case 0x04:
        printf((char*)cpu->ebx);
        break;
    default:
        break;
    }
    return esp;
}