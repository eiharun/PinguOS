#pragma once
#include <common/types.h>
#include <hardware_communication/port.h>
#include <hardware_communication/interrupts.h>
#include <drivers/driver.h>

using namespace common;
using namespace drivers;

namespace hardware_communication {

#define COMPUTE_ID(bus, device, function, register_offset) \
    (uint32_t)((bus<<16) | (device<<11) \
    | (function << 8) | (register_offset & 0xFC) \
    | (uint32_t)0x80000000);

#define COMMAND_PORT 0xCF8
#define DATA_PORT 0xCFC

#define NUM_BUS 8
#define NUM_DEVICES 32

// PCI Device Structure OFFSETS
#define VENDOR_ID_OFFSET 0x00
#define DEVICE_ID_OFFSET 0x02
#define CLASS_ID_OFFSET 0x0B
#define SUBCLASS_ID_OFFSET 0x0A
#define INTERFACE_ID_OFFSET 0x09
#define REVISION_OFFSET 0x08
#define HEADER_TYPE_OFFSET 0x0E
#define INTERRUPT_OFFSET 0x3C

class PCIDeviceDescriptor{
public:
    uint32_t port_base;
    uint32_t interrupt;

    uint8_t bus;
    uint8_t device;
    uint8_t function;

    uint16_t vendor_id;
    uint16_t device_id;

    uint8_t class_id;
    uint8_t subclass_id;
    uint8_t interface_id;

    uint8_t revision;

    PCIDeviceDescriptor(uint8_t bus, uint8_t device, uint8_t function);
};

class PCIController{
public:
    PCIController();
    ~PCIController();

    uint32_t read(uint8_t bus, uint8_t device, uint8_t function, uint8_t register_offset);
    void write(uint8_t bus, uint8_t device, uint8_t function, uint8_t register_offset, uint32_t value);
    bool device_has_functions(uint8_t bus, uint8_t device);
    void select_drivers(DriverManager* driver_manager);
    PCIDeviceDescriptor get_device_descriptor(uint8_t bus, uint8_t device, uint8_t function);
private:
    Port32 m_cmd_port;
    Port32 m_data_port;
};





} // Namespace hardware_communication