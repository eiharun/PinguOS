#pragma once
#include <drivers/driver.h>
#include <hardware_communication/interrupts.h>
#include <hardware_communication/pci.h>

using namespace hardware_communication;


namespace drivers {
    
// REGISTER OFFSETS
#define INTEL_82540_EM_STATUS_OFFSET 0x08
#define INTEL_82540_EM_EECD_OFFSET 0x10
#define INTEL_82540_EM_EERD_OFFSET 0x14
#define INTEL_82540_EM_CTRL_EXT_OFFSET 0x18
#define INTEL_82540_EM_MDI_CTRL_OFFSET 0x20
#define INTEL_82540_EM_ICR_OFFSET 0xC0
#define INTEL_82540_EM_ITR_OFFSET 0xC4
#define INTEL_82540_EM_ICS_OFFSET 0xC8
#define INTEL_82540_EM_IMS_OFFSET 0xD0
#define INTEL_82540_EM_IMC_OFFSET 0xD8
#define INTEL_82540_RCTL_OFFSET 0x0100
#define INTEL_82540_TCTL_OFFSET 0x0400
#define INTEL_82540_TIPG_OFFSET 0x0404
#define INTEL_82540_EM_RDBAL_OFFSET 0x2800
#define INTEL_82540_EM_RDBAH_OFFSET 0x2804
#define INTEL_82540_EM_RDLEN_OFFSET 0x2806
#define INTEL_82540_EM_RDH_OFFSET 0x2810
#define INTEL_82540_EM_RDT_OFFSET 0x2818
#define INTEL_82540_EM_TDH_OFFSET 0x3810
#define INTEL_82540_EM_TDT_OFFSET 0x3818
#define INTEL_82540_EM_TBAL_OFFSET 0x3800
#define INTEL_82540_EM_TBAH_OFFSET 0x3804
#define INTEL_82540_EM_TLEN_OFFSET 0x3808
#define INTEL_82540_EM_RAL_OFFSET 0x5400
#define INTEL_82540_EM_RAH_OFFSET 0x5404


// PHY Registers
#define PHY_CTRL_REG 0
#define PHY_STATUS_REG 1
#define PHY_ANA_REG 4

// Ring Buf Info
#define RX_RING_SIZE 256
#define TX_RING_SIZE 256

class Intel_82540EM: public drivers::Driver, hardware_communication::InterruptHandler{
public:
    Intel_82540EM(PCIDeviceDescriptor* dev, InterruptManager* interrupt);
    ~Intel_82540EM();

    void send_packet(uint8_t* data, uint16_t size);
    uint32_t handle_interrupt(uint32_t esp) override;
    void activate() override;
    int reset() override;
private:
    void get_mac_addr(bool v=true);
    bool init();
    uint16_t read_eeprom(uint8_t addr);
    bool write_phy(uint8_t reg, uint16_t data);
    uint32_t read_phy(uint8_t reg);
    
    void rx_setup();
    void tx_setup();
    
    void handle_rx();

    void enable_interrupts();
    int enable_bus_mastering();
    
    PCIDeviceDescriptor* m_dev;
    uint32_t m_reg_base;
    uint8_t m_mac_addr[6];

    struct RX_Descriptor{
        uint32_t buf_addr_lo; 
        uint32_t buf_addr_hi; 
        uint16_t len;
        uint16_t checksum;
        uint8_t status;
        uint8_t err;
        uint16_t special;
    }__attribute__((packed));
    
    struct TX_Descriptor{
        uint32_t buf_addr_lo; 
        uint32_t buf_addr_hi; 
        uint16_t len;
        uint8_t cso;
        uint8_t cmd;
        uint8_t status;
        uint8_t checksum;
        uint16_t special;
    }__attribute__((packed));

    RX_Descriptor* m_rx_ring;
    TX_Descriptor* m_tx_ring;

    Port32 m_pci_addr_port;
    Port32 m_pci_data_port;
};

}

