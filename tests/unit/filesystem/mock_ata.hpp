#include <drivers/ata.h>
#include <drivers/disk.h>

#include <vector>
// #include <cstdint>
#include <cstring>


class MockATA: public drivers::Disk{
public:
    static const size_t SECTOR_SIZE = 512;
    size_t m_sector_count;
    size_t DISK_SIZE = SECTOR_SIZE * m_sector_count;
    std::vector<uint8_t> m_data;

    MockATA() = default;

    MockATA(size_t sector_count): m_sector_count(sector_count){
        m_data.resize(DISK_SIZE);
        std::memset(m_data.data(), 0, DISK_SIZE);    
    }

    drivers::DiskErr identify() override {
        return drivers::DiskErr::SUCCESS;
    }

    drivers::DiskErr read_28(uint32_t sector, uint8_t* data, size_t count) override {
        if(count > SECTOR_SIZE)
            return drivers::DiskErr::INVALID_SIZE;
        if((sector * SECTOR_SIZE) + count >= DISK_SIZE){
            count = (sector * SECTOR_SIZE) - DISK_SIZE; //Do not overread a sector
        }
        std::memcpy(data, &m_data[sector * SECTOR_SIZE], count);
        
        return drivers::DiskErr::SUCCESS;
    }
    
    drivers::DiskErr write_28(uint32_t sector, uint8_t* data, size_t count) override {
        if(count > SECTOR_SIZE)
            return drivers::DiskErr::INVALID_SIZE;
        if((sector * SECTOR_SIZE) + count >= DISK_SIZE){
            return drivers::DiskErr::NO_SPACE;
        }
        std::memcpy(&m_data[sector * SECTOR_SIZE], data, count);
        // Fill remaining unwritten sector with 0
        std::memset(&m_data[(sector * SECTOR_SIZE)+count],0 ,SECTOR_SIZE-count);
        return drivers::DiskErr::SUCCESS;
    }

    drivers::DiskErr flush() override {
        return drivers::DiskErr::SUCCESS;
    }

};