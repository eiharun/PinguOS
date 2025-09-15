#pragma once
#include <common/types.h>
#include <gdt.h>
#include <hardware_communication/port.h>

class InterruptManager;

class InterruptHandler{
protected:
    uint8_t interrupt_number;
    InterruptManager* interrupt_manager;

    InterruptHandler(uint8_t interrupt_number, InterruptManager* interrupt_manager);
    ~InterruptHandler();
public:
    virtual uint32_t handle_interrupt(uint32_t esp);
};

class InterruptManager {
friend class InterruptHandler;
protected:
    static InterruptManager* ActiveInterruptManager;
    InterruptHandler* handlers[256];
    
    struct GateDescriptor{
        uint16_t handler_addr_lo;
        uint16_t gdt_code_segment_selector;
        uint8_t reserved;
        uint8_t access;
        uint16_t handler_addr_hi;
    }__attribute__((packed));

    static GateDescriptor interruptDescriptorTable[256];

    struct InterruptDescriptorTablePointer{
        uint16_t size;
        uint32_t base;
    }__attribute__((packed));

    static void set_interrupt_descriptor_table_entry(
        uint8_t interrupt_number,
        uint16_t code_segment_selector_offset,
        void (*handler)(),
        uint8_t descriptor_privilege_level,
        uint8_t descriptor_type
    );

    Port8_Slow pic_master_cmd;
    Port8_Slow pic_master_data;
    Port8_Slow pic_slave_cmd;
    Port8_Slow pic_slave_data;

public:
    InterruptManager(GlobalDescriptorTable* gdt);
    ~InterruptManager();
    void activate();
    void deactivate();
    static uint32_t handleInterrupt(uint8_t interrupt_number, uint32_t esp); 
    uint32_t handler(uint8_t interrupt_number, uint32_t esp); 
    
    static void ignoreInterruptRequest();
    static void handleInterruptRequest0x00(); // PIT
    static void handleInterruptRequest0x01(); // Keyboard
    static void handleInterruptRequest0x0C(); // Mouse
};

