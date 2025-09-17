#pragma once
#include <common/types.h>
#include <gdt.h>
#include <hardware_communication/port.h>

using namespace common;

namespace hardware_communication {

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

    Port8_Slow m_pic_master_cmd;
    Port8_Slow m_pic_master_data;
    Port8_Slow m_pic_slave_cmd;
    Port8_Slow m_pic_slave_data;

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
    static void handleInterruptRequest0x02();
    static void handleInterruptRequest0x03();
    static void handleInterruptRequest0x04();
    static void handleInterruptRequest0x05();
    static void handleInterruptRequest0x06();
    static void handleInterruptRequest0x07();
    static void handleInterruptRequest0x08();
    static void handleInterruptRequest0x09();
    static void handleInterruptRequest0x0A();
    static void handleInterruptRequest0x0B();
    static void handleInterruptRequest0x0C(); // Mouse
    static void handleInterruptRequest0x0D();
    static void handleInterruptRequest0x0E();
    static void handleInterruptRequest0x0F();
    static void handleInterruptRequest0x31();

    static void handleInterruptRequest0x80();

    static void handleException0x00();
    static void handleException0x01();
    static void handleException0x02();
    static void handleException0x03();
    static void handleException0x04();
    static void handleException0x05();
    static void handleException0x06();
    static void handleException0x07();
    static void handleException0x08();
    static void handleException0x09();
    static void handleException0x0A();
    static void handleException0x0B();
    static void handleException0x0C();
    static void handleException0x0D();
    static void handleException0x0E();
    static void handleException0x0F();
    static void handleException0x10();
    static void handleException0x11();
    static void handleException0x12();
    static void handleException0x13();
};

}
