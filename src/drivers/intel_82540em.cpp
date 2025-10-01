#include <drivers/intel_82540em.h>
#include <memory_management.h>
#include <common/macro.h>
#include <random>

void printf(char*);
void printf_hex32(uint32_t);
void printf_hex16(uint16_t);
void printf_hex(uint8_t);

Intel_82540EM::Intel_82540EM(PCIDeviceDescriptor* dev, InterruptManager* interrupt)
: Driver(), InterruptHandler(dev->interrupt + INTERRUPTS_BASE, interrupt),
  m_dev(dev){
    m_reg_base = dev->memory_base;
    // TODO: If not running on qemu, read MAC Addr from eeprom
    // uint16_t MAC0 = read_eeprom(0x00);
    // uint16_t MAC1 = read_eeprom(0x01);
    // uint16_t MAC2 = read_eeprom(0x02);
    // // Print bytes in network order (most sig first)
    // printf_hex(MAC2 & 0xFF); printf(":"); printf_hex((MAC2 >> 8) & 0xFF); printf(":");
    // printf_hex(MAC1 & 0xFF); printf(":"); printf_hex((MAC1 >> 8) & 0xFF); printf(":");
    // printf_hex(MAC0 & 0xFF); printf(":"); printf_hex((MAC0 >> 8) & 0xFF); printf("\n");
    // WRITE_REG(m_reg_base + INTEL_82540_EM_RAL_OFFSET, (MAC0 | (MAC1 << 16)));
    // WRITE_REG(m_reg_base + INTEL_82540_EM_RAH_OFFSET, (MAC2 | (1<<31))); // Set MAC2 and AV pin
    get_mac_addr(true);
    if(reset()){
        printf("Reset bit timeout\n");
    }
    init();
}

Intel_82540EM::~Intel_82540EM(){
    memory_management::MemoryManager::active_memory_manager->free(this);
}

void Intel_82540EM::get_mac_addr(bool v){
    uint32_t ral = READ_REG(m_reg_base + INTEL_82540_EM_RAL_OFFSET);
    uint32_t rah = READ_REG(m_reg_base + INTEL_82540_EM_RAH_OFFSET) & 0xFFFF;
    for(int i=0; i<6; ++i){
        m_mac_addr[i] = i<4 ? ((ral >> i*8) & 0xFF) : ((rah >> (i-4)*8) & 0xFF);
    }
    if(v){
        printf("MAC Address: ");
        for(int i=0; i<6; ++i){
            printf_hex(m_mac_addr[i]);
            if(i<5)
                printf(":");
        }
        printf("\n");
    }
}

int Intel_82540EM::reset(){
    /* CTRL.RST = 1
     * Poll wait for bit to turn back
     * To ensure that global device reset has fully completed and that
     * the Ethernet controller responds to subsequent access, wait
     * approximately 1 us after setting and before attempting to check
     * to see if the bit has cleared or to access any other device
     * register.
    */
    SET_REG(m_reg_base, (1<<26));
    for(int _=0; _<1000; ++_){} // wait 1us
    int reset{-1};
    for(int _=0; _<10000; ++_){
        if(!(READ_REG(m_reg_base) & (1<<26))){
            reset=0;
            break;
        }
    }
    return reset;
}

void Intel_82540EM::init(){
    /* CTRL.ASDE = 1
     * CTRL.SLU = 1
     * CTRL.FRCSPD = 0
    */
    SET_REG(m_reg_base, (3<<5) | (1<<11) );
    // Initialize PHY with ANA enabled
    // PHY Addr 001b
    // Set PHY ANA and PHY CTRL to enable ANA
    write_phy(PHY_CTRL_REG, /*TODO*/);
    write_phy(PHY_ANA_REG, /*TODO*/);
    reset();
    // Poll PHY STATUS for completion
    uint32_t status = read_phy(PHY_STATUS_REG);
}

void Intel_82540EM::write_phy(uint8_t reg, uint16_t& data){
    RESET_REG(m_reg_base + INTEL_82540_EM_MDI_CTRL_OFFSET, (1<<28)); //Clear Ready bit
    SET_REG(m_reg_base + INTEL_82540_EM_MDI_CTRL_OFFSET, data | (reg<<16) | (1<<21)/*PHY Addr*/ | (1<<26)/*WRITE OP*/);
    // Poll for ready bit (28) = 1 
    uint8_t ready = 0;
    for(int _=0; _<1000; ++_){
        ready = (READ_REG(m_reg_base + INTEL_82540_EM_MDI_CTRL_OFFSET) >> 28) & 0x01;
        if(ready){
            break;
        }
    }
    if(!ready){
        printf("Write phy timed out\n");
    }
}

uint32_t Intel_82540EM::read_phy(uint8_t reg){
    RESET_REG(m_reg_base + INTEL_82540_EM_MDI_CTRL_OFFSET, (1<<28)); //Clear Ready bit
    SET_REG(m_reg_base + INTEL_82540_EM_MDI_CTRL_OFFSET, (reg<<16) | (1<<21)/*PHY Addr*/ | (2<<26)/*READ OP*/);
    // Poll for ready bit (28) = 1 
    uint8_t ready = 0;
    for(int _=0; _<1000; ++_){
        ready = (READ_REG(m_reg_base + INTEL_82540_EM_MDI_CTRL_OFFSET) >> 28) & 0x01;
        if(ready){
            break;
        }
    }
    if(ready){
        uint32_t result = READ_REG(m_reg_base + INTEL_82540_EM_MDI_CTRL_OFFSET) & 0xFFFF; // DATA Bits
        return result;
    }
    else{
        printf("Read PHY timed out\n");
        return 0xFFFFFFFF;
    }
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
    // SET_REG(m_reg_base + INTEL_82540_EM_EECD_OFFSET, (1<<6));
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
    SET_REG(m_reg_base + INTEL_82540_EM_EERD_OFFSET, ((addr << 8) | 1));
    for(int i=0; i<100; ++i){
        uint32_t val = READ_REG(m_reg_base + INTEL_82540_EM_EERD_OFFSET);
        if(val & (1<<4)){
            // Write 0 to EEC.EE_REQ bit before returning
            // RESET_REG(m_reg_base + INTEL_82540_EM_EECD_OFFSET, (1<<6));
            return val>>16;
        } 
    }
    // Write 0 to EEC.EE_REQ bit
    // RESET_REG(m_reg_base + INTEL_82540_EM_EECD_OFFSET, (1<<6));
    printf("EEPROM read timeout");
    return 0;
}

void Intel_82540EM::activate(){

}