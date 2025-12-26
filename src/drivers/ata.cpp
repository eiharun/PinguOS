#include "common/types.h"
#include "drivers/disk.h"
#include "hardware_communication/interrupts.h"
#include <drivers/ata.h>

using namespace drivers;
void printf(char*);

ATA::ATA(InterruptManager* interrupt_manager,uint16_t port_base, bool master)
: InterruptHandler(ATA_INT, interrupt_manager),
m_bytes_per_sector(512), m_master(master),
m_data_port(port_base),
m_err_port(port_base + 1),
m_sector_count_port(port_base + 2),
m_lba_lo_port(port_base + 3),
m_lba_mid_port(port_base + 4),
m_lba_hi_port(port_base + 5),
m_drive_head_port(port_base + 6),
m_command_port(port_base + 7),
m_control_port(port_base == 0x1F0 ? 0x3F6 : 0x376)
// m_control_port(port_base + 0x206)
{
}

ATA::~ATA(){}

uint32_t ATA::handle_interrupt(uint32_t esp){
    return esp;
}

DiskErr ATA::identify(){
    /*
    To use the IDENTIFY command, select a target drive by sending 0xA0 for the master drive, or 0xB0 for the slave, to the "drive select" IO port. 
    On the Primary bus, this would be port 0x1F6. 
    Then set the Sectorcount, LBAlo, LBAmid, and LBAhi IO ports to 0 (port 0x1F2 to 0x1F5). 
    Then send the IDENTIFY command (0xEC) to the Command IO port (0x1F7). 
    Then read the Status port (0x1F7) again. If the value read is 0, the drive does not exist.
    For any other value: poll the Status port (0x1F7) until bit 7 (BSY, value = 0x80) clears.
    Because of some ATAPI drives that do not follow spec, at this point you need to check the LBAmid and LBAhi ports (0x1F4 and 0x1F5) to see if they are non-zero. 
    If so, the drive is not ATA, and you should stop polling. 
    Otherwise, continue polling one of the Status ports until bit 3 (DRQ, value = 8) sets, or until bit 0 (ERR, value = 1) sets.

    At that point, if ERR is clear, the data is ready to read from the Data port (0x1F0). 
    Read 256 16-bit values, and store them.
    */
    m_drive_head_port.write(m_master ? 0xA0 : 0xB0);

    m_sector_count_port.write(0x00);
    m_lba_lo_port.write(0x00);
    m_lba_mid_port.write(0x00);
    m_lba_hi_port.write(0x00);

    m_command_port.write(0xEC);
    uint32_t status = m_command_port.read();
    if(status == 0){
        printf("Drive DNE");
        return DiskErr::DNE;
    }
    while(((status & 0x80) == 0x80)){
        status = m_command_port.read();
    }
    uint32_t lba_mid = m_lba_mid_port.read();
    uint32_t lba_hi = m_lba_hi_port.read();
    if((lba_hi != 0) && (lba_mid != 0)){
        printf("Not ATA");
        return DiskErr::NOT_ATA;
    }
    while(((status & 0x08) == 0x08) && ((status & 0x01) == 0x01)){
        status = m_command_port.read();
    }
    if(status & 0x01){
        printf("ERROR");
        return DiskErr::OTHER;
    }
    
    for(uint16_t i=0; i<256; ++i){
        uint16_t data = m_data_port.read();
        char* foo = "  \0";
        foo[1] = (data >> 8) & 0x00FF;
        foo[0] = data & 0x00FF;
        printf(foo);
    }

    return DiskErr::SUCCESS;
}

DiskErr ATA::read_28(uint32_t sector, uint8_t* data, size_t count){
    if(sector & 0xF0000000)
        return DiskErr::INVALID_SECTOR;
    if(count > m_bytes_per_sector)
        return DiskErr::INVALID_SIZE;

    m_drive_head_port.write((m_master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    m_err_port.write(0);

    m_sector_count_port.write(0x01);
    m_lba_lo_port.write(sector & 0xFF);
    m_lba_mid_port.write((sector & 0xFF00) >> 8);
    m_lba_hi_port.write((sector & 0xFF0000) >> 16);

    m_command_port.write(0x20);
    
    uint8_t status = m_command_port.read();
    if(status == 0){
        printf("Drive DNE");
        return DiskErr::DNE;
    }
   
    while (status & 0x80)  // wait until BSY clears
        status = m_command_port.read();

    while (!(status & 0x08)) {  // wait until DRQ sets
        if (status & 0x01) {    // ERR
            printf("ERROR");
            return DiskErr::OTHER;
        }
        status = m_command_port.read();
    }

    if(status & 0x01){
        printf("ERROR");
        return DiskErr::OTHER;
    }
    
    // printf(" Reading from ATA ");
    // printf(" ");
    for(uint16_t i=0; i<count; i+=2){
        uint16_t w_data = m_data_port.read();
        // char* foo = "  \0";
        // foo[1] = (w_data >> 8) & 0x00FF;
        // foo[0] = w_data & 0x00FF;
        // printf(foo);
        data[i] = w_data & 0x00FF;
        if(i+1 < count)
            data[i+1] = (w_data >> 8) & 0x00FF;
    }
    for(uint16_t i=count + (count % 2); i<m_bytes_per_sector; i+=2){
        m_data_port.read();
    }
    Port8_Slow(0xA0).write(0x20); // Slave PIC EOI
    Port8_Slow(0x20).write(0x20); // Master PIC EOI
    return DiskErr::SUCCESS;
}

DiskErr ATA::write_28(uint32_t sector, uint8_t* data, size_t count){
    if(sector & 0xF0000000)
        return DiskErr::INVALID_SECTOR;
    if(count > m_bytes_per_sector)
        return DiskErr::INVALID_SIZE;

    m_drive_head_port.write((m_master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    m_err_port.write(0);

    m_sector_count_port.write(0x01);
    m_lba_lo_port.write(sector & 0xFF);
    m_lba_mid_port.write((sector & 0xFF00) >> 8);
    m_lba_hi_port.write((sector & 0xFF0000) >> 16);

    m_command_port.write(0x30);
    
    
    for(uint16_t i=0; i<count; i+=2){
        uint16_t w_data = data[i] ;
        if(i+1 < count)
            w_data |= (uint16_t)(data[i+1] << 8);
        else
            w_data |= 0x00;

        m_data_port.write(w_data);
    }
    for(uint16_t i=count + (count % 2); i<m_bytes_per_sector; i+=2){
        m_data_port.write(0x0000);
    }
    printf(" Written to ATA ");
    m_control_port.write(0x00);
    return DiskErr::SUCCESS;
}

DiskErr ATA::flush(){
    m_drive_head_port.write(m_master ? 0xE0 : 0xF0);
    m_command_port.write(0xE7);

    uint8_t status = m_command_port.read();
    while(((status & 0x80) == 0x80) && ((status & 0x01) == 0x01)){
        status = m_command_port.read();
    }
    if(status & 0x01){
        printf("ERROR");
        return DiskErr::OTHER;
    }
    return DiskErr::SUCCESS;
}

DiskErr ATA::reset(){
    m_drive_head_port.write(m_master ? 0xE0 : 0xF0);
    m_control_port.write(0x00);
}