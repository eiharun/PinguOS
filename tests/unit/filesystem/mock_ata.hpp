#include <drivers/ata.h>
#include <drivers/disk.h>

#include <vector>
// #include <cstdint>
#include <cstring>


class MockATA: public drivers::Disk{
public:
    static const size_t SECTOR_SIZE = 512;
    static constexpr size_t DISK_SIZE = SECTOR_SIZE * 128;
    std::vector<uint8_t> m_disk;

    MockATA(){
        m_disk.resize(DISK_SIZE);
        std::memset(m_disk.data(), 0, DISK_SIZE);    
    }

    drivers::DiskErr identify() override {
        return drivers::DiskErr::SUCCESS;
    }

    drivers::DiskErr read_28(uint32_t sector, uint8_t* data, size_t count) override {
        std::memcpy(data, &m_disk[sector * SECTOR_SIZE], count);
        return drivers::DiskErr::SUCCESS;
    }
    
    drivers::DiskErr write_28(uint32_t sector, uint8_t* data, size_t count) override {
        std::memcpy(&m_disk[sector * SECTOR_SIZE], data, count);
        return drivers::DiskErr::SUCCESS;
    }

    drivers::DiskErr flush() override {
        return drivers::DiskErr::SUCCESS;
    }

};