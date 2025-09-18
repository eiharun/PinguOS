#include <hardware_communication/pci.h>

using namespace hardware_communication;

PCIDeviceDescriptor::PCIDeviceDescriptor(uint8_t bus, uint8_t device, uint8_t function)
:bus(bus), device(device), function(function)
{}

PCIController::PCIController()
: m_cmd_port(COMMAND_PORT), 
  m_data_port(DATA_PORT)
{}

PCIController::~PCIController(){}

uint32_t PCIController::read(uint8_t bus, uint8_t device, uint8_t function, uint8_t register_offset){
    uint32_t id = COMPUTE_ID(bus, device, function, register_offset);
    m_cmd_port.write(id);
    uint32_t result = m_data_port.read();
    return result >> (register_offset & 2) * 8; 
}

void PCIController::write(uint8_t bus, uint8_t device, uint8_t function, uint8_t register_offset, uint32_t value){
    uint32_t id = COMPUTE_ID(bus, device, function, register_offset);
    m_cmd_port.write(id);
    m_data_port.write(value);
}

bool PCIController::device_has_functions(uint8_t bus, uint8_t device){
    // 7th bit of the header type field tells
    return read(bus, device, 0 , HEADER_TYPE_OFFSET) & (1<<7);
}

PCIDeviceDescriptor PCIController::get_device_descriptor(uint8_t bus, uint8_t device, uint8_t function){
    PCIDeviceDescriptor result(bus, device, function);
    result.vendor_id = read(bus, device, function, VENDOR_ID_OFFSET);
    result.device_id = read(bus, device, function, DEVICE_ID_OFFSET);
    result.class_id = read(bus, device, function, CLASS_ID_OFFSET);
    result.subclass_id = read(bus, device, function, SUBCLASS_ID_OFFSET);
    result.interface_id = read(bus, device, function, INTERFACE_ID_OFFSET);
    result.revision = read(bus, device, function, REVISION_OFFSET);
    result.interrupt = read(bus, device, function, INTERRUPT_OFFSET);
    return result;
}

void printf(int8_t* string);
void printf_hex(uint8_t value);

void PCIController::select_drivers(DriverManager* driver_manager){
    for(int bus=0; bus < NUM_BUS; ++bus){
        for(int device=0; device < NUM_DEVICES; ++device){
            int num_func = device_has_functions(bus, device) ? 8 : 1;
            for(int fn=0; fn < num_func; ++fn){
                PCIDeviceDescriptor dev = get_device_descriptor(bus, device, fn);
                if(dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF){
                    continue;
                }
                printf(" PCI BUS ");
                printf_hex(bus & 0xFF);
                printf(", DEVICE ");
                printf_hex(device & 0xFF);
                printf(", FUNCTION ");
                printf_hex(fn & 0xFF);

                printf(", VENDOR ID ");
                printf_hex((dev.vendor_id & 0xFF00) >> 8);
                printf_hex(dev.vendor_id & 0xFF);
                printf(", DEVICE ID ");
                printf_hex((dev.device_id & 0xFF00) >> 8);
                printf_hex(dev.device_id & 0xFF);
                printf("\n");
            }
        }
    }

}

