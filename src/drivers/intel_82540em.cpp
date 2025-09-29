#pragma once
#include <drivers/intel_82540em.h>
#include <memory_management.h>
#include <common/macro.h>

void printf(char*);
void printf_hex32(uint32_t);
void printf_hex16(uint16_t);
void printf_hex(uint8_t);

Intel_82540EM::Intel_82540EM(PCIDeviceDescriptor* dev, InterruptManager* interrupt)
: Driver(), InterruptHandler(dev->interrupt + INTERRUPTS_BASE, interrupt),
  m_dev(dev){
    m_reg_base = dev->memory_base;
    uint32_t ral = READ_REG(m_reg_base + INTEL_82540_EM_RAL_OFFSET);
    uint32_t rah = READ_REG(m_reg_base + INTEL_82540_EM_RAH_OFFSET) & 0xFFFF;
    // printf_hex32(ral);
    // printf_hex32(rah);
    printf("MAC Address: ");
    printf_hex(ral & 0xFF); printf(":"); printf_hex((ral >> 8) & 0xFF); printf(":");
    printf_hex((ral >> 16) & 0xFF); printf(":"); printf_hex((ral >> 24) & 0xFF);  printf(":");
    printf_hex(rah & 0xFF); printf(":"); printf_hex((rah >> 8) & 0xFF); printf("\n");
//     uint16_t MAC0 = read_eeprom(0x00);
//     uint16_t MAC1 = read_eeprom(0x01);
//     uint16_t MAC2 = read_eeprom(0x02);
//     // Print bytes in network order (most sig first)
//     printf_hex(MAC2 & 0xFF); printf(":"); printf_hex((MAC2 >> 8) & 0xFF); printf(":");
//     printf_hex(MAC1 & 0xFF); printf(":"); printf_hex((MAC1 >> 8) & 0xFF); printf(":");
//     printf_hex(MAC0 & 0xFF); printf(":"); printf_hex((MAC0 >> 8) & 0xFF); printf("\n");
//     WRITE_REG(m_reg_base + INTEL_82540_EM_RAL_OFFSET, (MAC0 | (MAC1 << 16)));
//     WRITE_REG(m_reg_base + INTEL_82540_EM_RAH_OFFSET, (MAC2 | (1<<31))); // Set MAC2 and AV pin
}

Intel_82540EM::~Intel_82540EM(){
    memory_management::MemoryManager::active_memory_manager->free(this);
}

uint16_t Intel_82540EM::read_eeprom(uint8_t addr){
    // bool eeprom_present = (READ_REG(m_reg_base + INTEL_82540_EM_EECD_OFFSET) & (1<<8));
    // if(eeprom_present){
    //     printf("EEPROM Present\n");
    // }
    // else{
    //     printf("EEPROM Not Present\n");
    // }

    // // Write 1 to EEC.EE_REQ bit
    // S_WRITE_REG(m_reg_base + INTEL_82540_EM_EECD_OFFSET, (1<<6));
    // // Read EEC.EE_GNT until it becomes 1 (stays 0 until access is granted)
    // bool granted{false};
    // for(int i = 0; i<100; ++i){
    //     uint8_t grant = READ_REG(m_reg_base + INTEL_82540_EM_EECD_OFFSET);
    //     if(grant & (1<<7)){
    //         granted=true;
    //         break;
    //     } 
    // }
    // if(!granted){
    //     printf("EEPROM Grant timout\n");
    //     return 0;
    // }
    S_WRITE_REG(m_reg_base + INTEL_82540_EM_EERD_OFFSET, ((addr << 8) | 1));
    for(int i=0; i<100; ++i){
        uint32_t val = READ_REG(m_reg_base + INTEL_82540_EM_EERD_OFFSET);
        if(val & (1<<4)){
            // Write 0 to EEC.EE_REQ bit before returning
            // R_WRITE_REG(m_reg_base + INTEL_82540_EM_EECD_OFFSET, (1<<6));
            return val>>16;
        } 
    }
    // Write 0 to EEC.EE_REQ bit
    // R_WRITE_REG(m_reg_base + INTEL_82540_EM_EECD_OFFSET, (1<<6));
    printf("EEPROM read timeout");
    return 0;
}

void Intel_82540EM::activate(){

}