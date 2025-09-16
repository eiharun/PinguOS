#include <hardware_communication/interrupts.h>
#include "gdt.h"

void printf(int8_t* string);

InterruptHandler::InterruptHandler(uint8_t interrupt_number, InterruptManager* interrupt_manager)
: interrupt_number(interrupt_number), interrupt_manager(interrupt_manager)
{
    interrupt_manager->handlers[interrupt_number] = this;
}

InterruptHandler::~InterruptHandler(){
    if(interrupt_manager->handlers[interrupt_number] == this){
        interrupt_manager->handlers[interrupt_number] = 0;
    }
}

uint32_t InterruptHandler::handle_interrupt(uint32_t esp){
    return esp;
}

InterruptManager::GateDescriptor InterruptManager::interruptDescriptorTable[256];

InterruptManager* InterruptManager::ActiveInterruptManager = 0;

void InterruptManager::set_interrupt_descriptor_table_entry(
    uint8_t interrupt_number,
        uint16_t code_segment_selector_offset,
        void (*handler)(),
        uint8_t descriptor_privilege_level,
        uint8_t descriptor_type
){
    interruptDescriptorTable[interrupt_number].handler_addr_lo = ((uint32_t) handler) & 0xFFFF;
    interruptDescriptorTable[interrupt_number].handler_addr_hi = (((uint32_t) handler) >> 16) & 0xFFFF;
    interruptDescriptorTable[interrupt_number].gdt_code_segment_selector = code_segment_selector_offset;

    const uint8_t IDT_DESC_PRESENT = 0x80;
    interruptDescriptorTable[interrupt_number].access = IDT_DESC_PRESENT | ((descriptor_privilege_level & 3) << 5) | descriptor_type;
    interruptDescriptorTable[interrupt_number].reserved = 0;

}

InterruptManager::InterruptManager(GlobalDescriptorTable* gdt)
: m_pic_master_cmd(0x20),
  m_pic_master_data(0x21),
  m_pic_slave_cmd(0xA0),
  m_pic_slave_data(0xA1){
    uint16_t code_segment = gdt->codeSegmentSelector();
    const uint8_t IDT_INTERRUPT_GATE = 0xE;

    for(uint16_t i=0; i<256; ++i){
        handlers[i] = 0;
        set_interrupt_descriptor_table_entry(i, code_segment, &ignoreInterruptRequest, 0, IDT_INTERRUPT_GATE);
    }

    set_interrupt_descriptor_table_entry(0x20, code_segment, &handleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x21, code_segment, &handleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x2C, code_segment, &handleInterruptRequest0x0C, 0, IDT_INTERRUPT_GATE);
  

    m_pic_master_cmd.write(0x11);
    m_pic_slave_cmd.write(0x11);

    m_pic_master_data.write(0x20);
    m_pic_slave_data.write(0x28);

    m_pic_master_data.write(0x04);
    m_pic_slave_data.write(0x02);

    m_pic_master_data.write(0x01);
    m_pic_slave_data.write(0x01);

    m_pic_master_data.write(0x00);
    m_pic_slave_data.write(0x00);

    InterruptDescriptorTablePointer idt;
    idt.size = 256 * sizeof(GateDescriptor)-1;
    idt.base = (uint32_t)interruptDescriptorTable;
    asm volatile("lidt %0" : : "m" (idt));
}
InterruptManager::~InterruptManager(){}

void InterruptManager::activate(){
    if(ActiveInterruptManager != 0){
        ActiveInterruptManager->deactivate();
    }
    ActiveInterruptManager = this;
    asm("sti");
}

void InterruptManager::deactivate(){
    if(ActiveInterruptManager == this){
        ActiveInterruptManager = 0;
        asm("cli");
    }
}

uint32_t InterruptManager::handleInterrupt(uint8_t interrupt_number, uint32_t esp){
    if(ActiveInterruptManager != 0){
        return ActiveInterruptManager->handler(interrupt_number, esp);
    }
    return esp;
}

uint32_t InterruptManager::handler(uint8_t interrupt_number, uint32_t esp){
    if(handlers[interrupt_number] != 0){
        esp = handlers[interrupt_number]->handle_interrupt(esp);
    }
    else if(interrupt_number != 0x20){
        char* foo = "UNHANDLED INTERRUPT 0x00";
        char* hex = "0123456789ABCDEF";
        foo[12] = hex[(interrupt_number >> 4) & 0x0F];
        foo[13] = hex[interrupt_number & 0x0F];
        printf(foo);
    }
    
    // Send EOI
    if(0x20 <= interrupt_number && interrupt_number < 0x30){
        if(0x28 <= interrupt_number){ // slave ints
            m_pic_slave_cmd.write(0x20);
        }
        m_pic_master_cmd.write(0x20);
    }
    
    return esp;
}


