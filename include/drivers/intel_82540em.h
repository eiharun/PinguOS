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

class Intel_82540EM: public drivers::Driver, hardware_communication::InterruptHandler{
public:
    Intel_82540EM(PCIDeviceDescriptor* dev, InterruptManager* interrupt);
    ~Intel_82540EM();

    void activate() override;
private:
    uint16_t read_eeprom(uint8_t addr);

    PCIDeviceDescriptor* m_dev;
    uint32_t m_reg_base;
};

}

