#include "interrupts.h"
#include "gdt.h"

void printf(int8_t* string);

InterruptManager::GateDescriptor InterruptManager::interruptDescriptorTable[256];

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
: pic_master_cmd(0x20),
  pic_master_data(0x21),
  pic_slave_cmd(0xA0),
  pic_slave_data(0xA1){
    uint16_t code_segment = gdt->codeSegmentSelector();
    const uint8_t IDT_INTERRUPT_GATE = 0xE;

    for(uint16_t i=0; i<256; ++i){
        set_interrupt_descriptor_table_entry(i, code_segment, &ignoreInterruptRequest, 0, IDT_INTERRUPT_GATE);
    }

    set_interrupt_descriptor_table_entry(0x20, code_segment, &handleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x21, code_segment, &handleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);
  

    pic_master_cmd.write(0x11);
    pic_slave_cmd.write(0x11);

    pic_master_data.write(0x20);
    pic_slave_data.write(0x28);

    pic_master_data.write(0x04);
    pic_slave_data.write(0x02);

    pic_master_data.write(0x01);
    pic_slave_data.write(0x01);

    pic_master_data.write(0x00);
    pic_slave_data.write(0x00);

    // mask all IRQs (master and slave) while debugging
   


    InterruptDescriptorTablePointer idt;
    idt.size = 256 * sizeof(GateDescriptor)-1;
    idt.base = (uint32_t)interruptDescriptorTable;
    asm volatile("lidt %0" : : "m" (idt));
}
InterruptManager::~InterruptManager(){}

void InterruptManager::activate(){
    asm("sti");
}

uint32_t InterruptManager::handleInterrupt(uint8_t interrupt_number, uint32_t esp){
    printf("  INTERRUPT");
    return esp;
}
