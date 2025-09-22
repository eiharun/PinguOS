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

void PCIController::select_drivers(DriverManager* driver_manager, InterruptManager* interrupt){
    for(int bus=0; bus < NUM_BUS; ++bus){
        for(int device=0; device < NUM_DEVICES; ++device){
            int num_func = device_has_functions(bus, device) ? 8 : 1;
            for(int fn=0; fn < num_func; ++fn){
                PCIDeviceDescriptor dev = get_device_descriptor(bus, device, fn);
                if(dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF){
                    continue;
                }

                for(int bar_num=0; bar_num < NUM_BARS; ++bar_num){
                    BaseAddressRegister bar = get_base_address_register(bus, device, fn, bar_num);
                    if((bar.addr!=0) && (bar.type==BAR::IO)){
                        dev.port_base = (uint32_t)bar.addr;
                    }

                    Driver* driver = get_driver(dev, interrupt);
                    if(driver!=0){
                        driver_manager->add_driver(driver);
                    }

                }


                printf(" BUS ");
                printf_hex(bus & 0xFF);
                printf(", DEV ");
                printf_hex(device & 0xFF);
                printf(", FN ");
                printf_hex(fn & 0xFF);

                printf(", VENDOR ID ");
                printf_hex((dev.vendor_id & 0xFF00) >> 8);
                printf_hex(dev.vendor_id & 0xFF);
                printf(", DEVICE ID ");
                printf_hex((dev.device_id & 0xFF00) >> 8);
                printf_hex(dev.device_id & 0xFF);
                printf("\n");
                
                printf(", CLASS ID ");
                printf_hex((dev.class_id & 0xFF00 ) >> 8);
                printf_hex(dev.class_id & 0xFF);
                printf(", SUBCLASS ID ");
                printf_hex((dev.subclass_id & 0xFF00 ) >> 8);
                printf_hex(dev.subclass_id & 0xFF);

                printf("\n");
            }
        }
    }

}

BaseAddressRegister PCIController::get_base_address_register(uint8_t bus, uint8_t device, uint8_t function, uint8_t bar_num){
    BaseAddressRegister result;

    uint32_t header_type = read(bus, device, function, HEADER_TYPE_OFFSET) & 0x7F;
    int max_bars = 6 - (4 * header_type);
    if(bar_num >= max_bars){
        return result;
    }

    uint32_t bar_value = read(bus, device, function, BAR_START_OFFSET + 4*bar_num);
    result.type = (bar_value & 0x01) ? BAR::IO : BAR::MM;
    uint32_t temp;

    switch(result.type){
        case BAR::MM:
            result.prefetchable = (((bar_value >> 3) & 0x01) == 0x01);
            switch((MemoryMapMode)((bar_value >> 1) & 0x3)){
                case MMMode::B32: // 32bit mode
                    break;
                case MMMode::B20: // 20bit mode
                    break;
                case MMMode::B64: // 64bit mode
                    break;
            }
            break;
        case BAR::IO:
            result.addr = (uint8_t*)(bar_value & ~0x3);
            result.prefetchable = false;
            break;
    }

    return result;
}

Driver* PCIController::get_driver(PCIDeviceDescriptor dev, InterruptManager* interrupt){
    // TODO Add enum for cases (not yet, since a better option may be to have a lookup table for drivers)
    switch(dev.vendor_id){
        case 0x1022: // AMD
            // switch(dev.device_id){}
            break;
        case 0x8086: // Intel
            switch(dev.device_id){
                case 0x100E: // Intel 82540EM
                    printf("Intel 82540EM ");
                    break;
            }
            break;
    }

    switch(dev.class_id){
        case 0x00: // Pre PCI 2.0
            switch(dev.subclass_id){
                case 0x00: break;
                case 0x01: // VGA
                    printf("Pre PCI 2.0 VGA ");
                    break;
            }
        case 0x03: // Graphics
            switch(dev.subclass_id){
                case 0x00: // VGA
                    printf("VGA ");
                    break;
            }
            break;
    }

    return 0;
}