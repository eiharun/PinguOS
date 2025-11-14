#include <drivers/ata.h>
#include <drivers/disk.h>

#include <vector>
// #include <cstdint>
#include <cstring>


class MockATA: public drivers::Disk<drivers::ATAError>{
public:
    static const size_t SECTOR_SIZE = 512;
    static constexpr size_t DISK_SIZE = SECTOR_SIZE * 128;
    std::vector<uint8_t> m_disk;

    MockATA(){
        m_disk.resize(DISK_SIZE);
        std::memset(m_disk.data(), 0, DISK_SIZE);    
    }

    drivers::ATAError identify() override {
        return drivers::ATAError::SUCCESS;
    }

    drivers::ATAError read_28(uint32_t sector, uint8_t* data, size_t count) override {
        std::memcpy(data, &m_disk[sector * SECTOR_SIZE], count);
        return drivers::ATAError::SUCCESS;
    }
    
    drivers::ATAError write_28(uint32_t sector, uint8_t* data, size_t count) override {
        std::memcpy(&m_disk[sector * SECTOR_SIZE], data, count);
        return drivers::ATAError::SUCCESS;
    }

    drivers::ATAError flush() override {
        return drivers::ATAError::SUCCESS;
    }

};