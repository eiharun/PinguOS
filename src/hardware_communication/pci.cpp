#include "memory_management.h"
#include <hardware_communication/pci.h>
#include <drivers/intel_82540em.h>
#include <common/macro.h>

using namespace hardware_communication;

PCIDeviceDescriptor::PCIDeviceDescriptor(uint8_t bus, uint8_t device, uint8_t function)
:bus(bus), device(device), function(function), port_base(0), memory_base(0), memory_size(0)
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
    result.status = read(bus, device, function, STATUS_OFFSET);
    result.command = read(bus, device, function, COMMAND_OFFSET);
    result.revision = read(bus, device, function, REVISION_OFFSET);
    result.interrupt = read(bus, device, function, INTERRUPT_OFFSET);
    return result;
}

void printf(int8_t*);
void printf_hex(uint8_t);
void printf_hex32(uint32_t);

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
                    if((bar.addr!=0) && (bar.size!=0)){
                        if(bar.type==BAR::IO){
                            dev.type = BAR::IO;
                            dev.port_base = (uint32_t)bar.addr;
                        }
                        else if(bar.type==BAR::MM && dev.memory_base==0){
                            dev.type = BAR::MM;
                            dev.memory_base = (uint32_t)bar.addr;
                            dev.memory_size = bar.size;
                        }
                    }

                }

                Driver* driver = get_driver(dev, interrupt);
                if(driver!=0){
                    driver_manager->add_driver(driver);
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
                
                // printf(", CLASS ID ");
                // printf_hex((dev.class_id & 0xFF00 ) >> 8);
                // printf_hex(dev.class_id & 0xFF);
                // printf(", SUBCLASS ID ");
                // printf_hex((dev.subclass_id & 0xFF00 ) >> 8);
                // printf_hex(dev.subclass_id & 0xFF);

                // printf("\n");
            }
        }
    }

}

BaseAddressRegister PCIController::get_base_address_register(uint8_t bus, uint8_t device, uint8_t function, uint8_t bar_num){
    BaseAddressRegister result;
    result.addr = 0;
    result.size = 0;
    result.prefetchable = false;

    uint32_t header_type = read(bus, device, function, HEADER_TYPE_OFFSET) & 0x7F;
    int max_bars = 6 - (4 * header_type);
    if(bar_num >= max_bars){
        return result;
    }
    const uint32_t bar_offset = BAR_START_OFFSET + 4 * bar_num;
    uint32_t bar_value = read(bus, device, function, bar_offset);
    result.type = (bar_value & 0x01) ? BAR::IO : BAR::MM;
    
    switch(result.type){
        case BAR::MM:{
            write(bus, device, function, bar_offset, 0xFFFFFFFF);
            uint32_t size_mask = read(bus, device, function, bar_offset);
            write(bus, device, function, bar_offset, bar_value);

            result.prefetchable = (((bar_value >> 3) & 0x01) == 0x01);
            MMMode mode = (MemoryMapMode)((bar_value >> 1) & 0x3); 
            switch(mode){
                case MMMode::B64:{ // 64bit mode
                    // printf("64-bit BAR size detected\n");
                    const uint32_t bar_hi_offset = bar_offset + 4;
                    // Read the Hi 32-bits of the 64-bit BAR
                    uint32_t bar_hi_addr = read(bus, device, function, bar_hi_offset);
                    if(bar_hi_addr != 0){
                        // Means address > 4gb, which is not possible on 32-bit system (without PAE)
                        printf("Warning: 64-bit BAR >4GB, skipping...\n");
                        result.addr = 0;
                        result.size = 0;
                    }
                    else{
                        // Address < 4gb, safe on 32-bit
                        result.addr = (uint8_t*)(bar_value & ~0x0F);

                        write(bus, device, function, bar_hi_offset, 0xFFFFFFFF);
                        uint32_t size_mask_high = read(bus, device, function, bar_hi_offset);
                        write(bus, device, function, bar_hi_offset, bar_hi_addr);

                        if(size_mask_high != 0xFFFFFFFF){
                            // Size can potentially be greater than 4gb
                            printf("Warning: 64-bit BAR size may exceed 4GB; using low approx on 32-bit.\n");
                        }
                        result.size = (~(size_mask & ~0x0F) + 1);
                    }

                    break;
                }
                case MMMode::B32: // 32bit mode
                    // printf(" 32-bit BAR ");
                case MMMode::B20: // 20bit mode
                    // printf(" 20-bit BAR ");
                    result.addr = (uint8_t*)(bar_value & ~0x0F);
                    result.size = (~(size_mask & ~0x0F) + 1);
                    break;
            }
            break;
        }
        case BAR::IO:
            result.addr = (uint8_t*)(bar_value & ~0x3);
            result.prefetchable = false;
            break;
    }

    return result;
}

Driver* PCIController::get_driver(PCIDeviceDescriptor dev, InterruptManager* interrupt){
    // TODO Add enum for cases (not yet, since a better option may be to have a lookup table for drivers)
    Driver* driver = 0;
    switch(dev.vendor_id){
        case 0x1022: // AMD
            // switch(dev.device_id){}
            break;
        case 0x8086: // Intel
            switch(dev.device_id){
                case 0x100E: // Intel 82540EM
                    // printf("")
                    printf("Intel 82540EM");
                    printf(" - Memory Base: 0x");
                    printf_hex32(dev.memory_base);
                    printf(", Size: 0x");
                    printf_hex32(dev.memory_size);
                    printf("\n");
                    // SET bit 2 in command register to 1 to enable bus mastering (allows DMA to function normally)
                    if(!(dev.command & 0x06)){
                        write(dev.bus, dev.device, dev.function, COMMAND_OFFSET, dev.command | 0x06);
                    }

                    // uint32_t status = READ_REG(dev.memory_base + INTEL_82540_EM_STATUS_OFFSET);
                    // printf_hex32(status);
                    // if(( status & (1<<1) )){ // read 2nd bit of status reg
                    //     printf(" Error: TBI present\n"); // Make sure TBI is not present since the device does not support it
                    // }
                    // else{
                    // }
                    printf("Instantiating Driver ");
                    driver = (Intel_82540EM*)memory_management::MemoryManager::active_memory_manager->malloc(sizeof(Intel_82540EM));
                    if(driver != 0){
                        new (driver) Intel_82540EM(&dev,interrupt);
                    }
                    else{
                        printf(" Could not allocate memory\n");
                    }
                    return driver;
                    break;
            }
            break;
    }

    switch(dev.class_id){
        case 0x00: // Pre PCI 2.0
            switch(dev.subclass_id){
                case 0x00: break;
                case 0x01: // VGA
                    printf("Pre PCI 2.0 VGA: ");
                    break;
            }
            break;
        case 0x03: // Graphics
            switch(dev.subclass_id){
                case 0x00: // VGA
                    printf("VGA: ");
                    break;
            }
            break;
    }

    return 0;
}