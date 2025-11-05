#pragma once
#include "filesystem/fat.h"
#include <common/types.h>
#include <drivers/ata.h>

using namespace common;
using namespace drivers;

namespace filesystem {

struct PartitionTableEntry{
    uint8_t bootable;

    uint8_t start_head;
    uint8_t start_sector: 6;
    uint16_t start_cylinder: 10;
    
    uint8_t partition_id;
    
    uint8_t end_head;
    uint8_t end_sector: 6;
    uint16_t end_cylinder: 10;
    
    uint32_t start_lba;
    uint32_t len;
}__attribute__((packed));

struct MasterBootRecord{
    uint8_t bootloader[440];
    uint32_t sig;
    uint16_t unused;

    PartitionTableEntry primary_partition[4];

    uint16_t magic_number;
}__attribute__((packed));

enum class MSDOSERR {SUCCESS=0, PARTITION_DOES_NOT_EXIST};

class MSDOSPartitionTable{
public:
    MSDOSPartitionTable(ATA* hd);
    static void read_partitions(ATA* hd);
    MSDOSERR get_partition_offset(uint32_t& offset, int partition_number) ;
private:
    MasterBootRecord m_mbr;
};

}