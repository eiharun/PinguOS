#pragma once
#include <drivers/driver.h>
#include <hardware_communication/interrupts.h>
#include <hardware_communication/pci.h>

using namespace hardware_communication;

// REGISTER OFFSETS

namespace drivers {

#define INTEL_82540_EM_STATUS_OFFSET 0x08
#define INTEL_82540_EM_EECD_OFFSET 0x10
#define INTEL_82540_EM_EERD_OFFSET 0x14
#define INTEL_82540_EM_CTRL_EXT_OFFSET 0x18
#define INTEL_82540_EM_MDI_CTRL_OFFSET 0x20
#define INTEL_82540_EM_RAL_OFFSET 0x5400
#define INTEL_82540_EM_RAH_OFFSET 0x5404

// PHY Registers
#define PHY_CTRL_REG 0
#define PHY_STATUS_REG 1
#define PHY_ANA_REG 4


class Intel_82540EM: public drivers::Driver, hardware_communication::InterruptHandler{
public:
    Intel_82540EM(PCIDeviceDescriptor* dev, InterruptManager* interrupt);
    ~Intel_82540EM();

    void activate() override;
    int reset() override;
private:
    void get_mac_addr(bool v);
    void init();
    uint16_t read_eeprom(uint8_t addr);
    void write_phy(uint8_t reg, uint16_t& data);
    uint32_t read_phy(uint8_t reg);

    PCIDeviceDescriptor* m_dev;
    uint32_t m_reg_base;
    uint8_t m_mac_addr[6];
};

}

