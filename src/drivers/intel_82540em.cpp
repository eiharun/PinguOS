#include "common/types.h"
#include <drivers/intel_82540em.h>
#include <memory_management.h>
#include <common/macro.h>

using namespace memory_management;

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
    rx_setup();
    tx_setup();

}

Intel_82540EM::~Intel_82540EM(){
    MemoryManager::active_memory_manager->free(this);
}

void Intel_82540EM::send_packet(uint8_t* data, uint16_t size){
    uint32_t tdt = READ_REG(m_reg_base + INTEL_82540_EM_TDT_OFFSET); 
    TX_Descriptor* desc = &m_tx_ring[tdt];

    if (!(desc->status & 0x1)) {
        printf("TX ring full!\n");
        return;
    }

    uint8_t* tx_buf = (uint8_t*)(desc->buf_addr_lo);
    // deep copy
    for(uint16_t i=0; i<size; ++i){
        tx_buf[i] = data[i];
    }

    desc->len = size;
    desc->cmd_cso = 1 | (1<<3);
    desc->status = 0;
    
    tdt = (tdt+1) % 16;
    WRITE_REG(m_reg_base + INTEL_82540_EM_TDT_OFFSET, tdt); 
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

bool Intel_82540EM::init(){
    /* CTRL.ASDE = 1
     * CTRL.SLU = 1
     * CTRL.FRCSPD = 0
    */
    SET_REG(m_reg_base, (3<<5) | (1<<11) );
    // Initialize PHY with ANA enabled
    // PHY Addr 001b
    // Set PHY ANA and PHY CTRL to enable ANA
    if(!write_phy(PHY_CTRL_REG, ((1<<9) | (1<<12) | (1<<15)))){
        printf("Write failed\n");
    }
    if(!write_phy(PHY_ANA_REG, 0x0DE1)){
        printf("Write failed\n");
    }
    // Poll PHY STATUS for completion
    bool good_status{false};
    uint32_t status{};
    uint32_t mac_status = READ_REG(m_reg_base + INTEL_82540_EM_STATUS_OFFSET);

    
    for(int _=0; _<10000; ++_){
        status = read_phy(PHY_STATUS_REG);
        if(status & (1<<5)){
            // Link status & Auto-negotiation completion
            // Would normally also check link-up here too (PHY or MAC status)(but QEMU doesn't seem to set the bit)
            good_status=true;
            break;
        }
    }
    if(!good_status){
        printf("Auto-negotiation failed\n"); // 0x41E0 
        printf_hex32(status);
    }
    
    return good_status;
}

void Intel_82540EM::enable_interrupts(){
    // Clear ICR
    WRITE_REG(INTEL_82540_EM_ICR_OFFSET, 0);
    // Use IMS to enable LCS(2) RXT0(7) TXDW(0)
    SET_REG(INTEL_82540_EM_IMS_OFFSET, 1 | (1<<2) | (1<<7));
}

uint32_t Intel_82540EM::handle_interrupt(uint32_t esp){
    // Read ICR
    uint16_t interrupts = READ_REG(INTEL_82540_EM_ICR_OFFSET) & 0xFFFF;
    printf("Some interrupt\n");
    // Handle interrupts
    if(interrupts & (1<<7)){ 
        // RX 
        printf("RX Interrupt ");
    }
    if(interrupts & (1<<2)){
        // Link status change
        printf("Link status interrupt ");
    }
    if(interrupts & (1<<0)){
        // TX Complete
        printf("Transmission Complete ");
    }
    
    // // Clear ICR
    // WRITE_REG(INTEL_82540_EM_ICR_OFFSET, 0);

    return esp;
}

void Intel_82540EM::rx_setup(){
    m_rx_ring = (RX_Descriptor*)MemoryManager::active_memory_manager->malloc(RX_RING_SIZE * sizeof(RX_Descriptor), 16);
    if(!m_rx_ring){
        printf("RX ring alloc failed\n");
        return;
    }

    for(int i=0; i<RX_RING_SIZE; ++i){
        void* buffer = MemoryManager::active_memory_manager->malloc(2048);
        if(!buffer){
            printf("RX Buffer alloc failed\n");
            return;
        }
        m_rx_ring[i].buf_addr_lo = (uint32_t)buffer;
        m_rx_ring[i].buf_addr_hi = 0;
        m_rx_ring[i].len = 0;
        m_rx_ring[i].err_status = 0;
        m_rx_ring[i].checksum = 0;
        m_rx_ring[i].special = 0;
    }
    /* RX Ring setup complete */
    // TODO Set registers
    /*
        Set RDBAL RDBAH
        Set RDH and RDT registers with buffer head and tail(one descriptor beyond last valid descriptor)
        RCTL.EN = 1 (After recieve descriptor ring is initialized and software is ready to process packets)
        RCTL.LPE = 0 
        RCTL.LBM = 00b
        RCTL.BAM = 1
        RCTL.BSIZE = 00b [2048]
    */
    //RDBAL RDBAH   
    WRITE_REG(INTEL_82540_EM_RDBAL_OFFSET, (uint32_t)m_rx_ring);
    WRITE_REG(INTEL_82540_EM_RDBAH_OFFSET, 0);
    //SET RDH and RDT and RDLEN
    WRITE_REG(INTEL_82540_EM_RDLEN_OFFSET, RX_RING_SIZE * sizeof(RX_Descriptor));
    WRITE_REG(INTEL_82540_EM_RDH_OFFSET, 0); // First Descriptor
    WRITE_REG(INTEL_82540_EM_RDT_OFFSET, RX_RING_SIZE-1); // Last Descriptor

    RESET_REG(INTEL_82540_RCTL_OFFSET, (1<<5) | (2<<6) | (2<<16));
    SET_REG(INTEL_82540_RCTL_OFFSET, (1<1) | (1<<15));
    printf(" RX Setup Complete ");
}

void Intel_82540EM::tx_setup(){
    m_tx_ring = (TX_Descriptor*)MemoryManager::active_memory_manager->malloc(TX_RING_SIZE * sizeof(TX_Descriptor), 16);
    if(!m_tx_ring){
        printf("TX ring alloc failed\n"); 
        return;
    }

    for(int i=0; i<RX_RING_SIZE; ++i){
        void* buffer = MemoryManager::active_memory_manager->malloc(2048);
        if(!buffer){
            printf("TX Buffer alloc failed\n");
            return;
        }
        m_tx_ring[i].buf_addr_lo = (uint32_t)buffer;
        m_tx_ring[i].buf_addr_hi = 0;
        m_tx_ring[i].len = 0;
        m_tx_ring[i].status = 0;
        m_tx_ring[i].cmd_cso = 0;
        m_tx_ring[i].checksum = 0;
        m_tx_ring[i].special = 0;
    }
    /* TX Ring setup complete */
    // TODO Set registers
    /*
        TDBAL/TDBAH to trasmit descriptor base addr
        Set TDH and TDT 0
        TDLEN = sizeof(descriptor_ring) * TX_RING_SIZE
        TCTL.EN = 1
        TCTL.PSP = 1
    */
    // Set TDBAL/H/LEN
    WRITE_REG(INTEL_82540_EM_TBAL_OFFSET, (uint32_t)m_tx_ring);
    WRITE_REG(INTEL_82540_EM_TBAH_OFFSET, 0);
    WRITE_REG(INTEL_82540_EM_TLEN_OFFSET, TX_RING_SIZE * sizeof(TX_Descriptor));
    // TDH & TDT
    WRITE_REG(INTEL_82540_EM_TDH_OFFSET, 0);
    WRITE_REG(INTEL_82540_EM_TDT_OFFSET, 0);
    SET_REG(INTEL_82540_TCTL_OFFSET, (1<<1) | (1<<3));
    printf(" TX Setup Complete ");
}

bool Intel_82540EM::write_phy(uint8_t reg, uint16_t data){
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
    return (READ_REG(m_reg_base + INTEL_82540_EM_MDI_CTRL_OFFSET) >> 30) & 0x1 ? true : false;
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
    enable_interrupts();
}