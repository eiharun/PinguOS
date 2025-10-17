#pragma once
#include <common/types.h>
#include <hardware_communication/interrupts.h>
#include <multitask.h>

using namespace common;

class SyscallHandler: public hardware_communication::InterruptHandler{
public:
    SyscallHandler(hardware_communication::InterruptManager* interrupt_manager, uint8_t interrupt_number);
    ~SyscallHandler();
    uint32_t handle_interrupt(uint32_t esp);
protected:

};