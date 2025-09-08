#pragma once
#include "types.h"
#include "gdt.h"

class InterruptManager {
protected:
    struct GateDescriptor{
        uint16_t handle_addr_lo;
        uint16_t gdt_code_segment_selector;
        uint8_t reserved;
        uint8_t access;
        uint16_t handler_addr_hi;
    }__attribute__((packed));

    static GateDescriptor interruptDescriptorTable[256];

    static void setInterruptDescriptorTableEntry(
        uint8_t interrupt_number,
        uint16_t code_segment_selector_offset,
        void (*handler)(),
        uint8_t descriptor_privilege_level,
        uint8_t descriptor_type
    );

public:
    InterruptManager(GlobalDescriptorTable* gdt);
    ~InterruptManager();
    static uint32_t handleInterrupt(uint8_t interrupt_number, uint32_t esp); 
    static void ignoreInterruptRequest();
    static void handleInterruptRequest0x00();
    static void handleInterruptRequest0x01();
};