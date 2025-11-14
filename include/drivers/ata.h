#pragma once
#include <drivers/disk.h>
#include <hardware_communication/port.h>

using namespace hardware_communication;

namespace drivers {

class ATA : public Disk{
protected:
    Port16 m_data_port;
    Port8 m_err_port;
    Port8 m_sector_count_port;
    Port8 m_lba_lo_port;
    Port8 m_lba_mid_port;
    Port8 m_lba_hi_port;
    Port8 m_drive_head_port;
    Port8 m_command_port;
    Port8 m_control_port;
    bool m_master;
    uint16_t m_bytes_per_sector;
public:
    ATA(uint16_t port_base, bool master);
    ~ATA();

    DiskErr identify() override;
    DiskErr read_28(uint32_t sector, uint8_t* data, size_t count) override;
    DiskErr write_28(uint32_t sector, uint8_t* data, size_t count) override;
    DiskErr flush() override;
};

}