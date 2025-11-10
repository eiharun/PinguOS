#include "common/types.h"
#include "filesystem/filesystem.h"
#include <filesystem/msdospart.h>
#include <filesystem/fat.h>

using namespace filesystem;

void printf(char*);
void printf_hex(uint8_t);
void printf_hex32(uint32_t);
void MSDOSPartitionTable::read_partitions(ATA* hd){
    MasterBootRecord mbr;
    hd->read_28(0, (uint8_t*)&mbr, sizeof(MasterBootRecord));
    if(mbr.magic_number != 0xAA55){
        printf("Illegal MBR!");
        return;
    }

    for(int i=0; i<4; ++i){
        if(mbr.primary_partition[i].partition_id == 0x00)
            continue;
        
        printf(" Partition: ");
        printf_hex(i);
        if(mbr.primary_partition[i].bootable == 0x80){
            printf(" Bootable, Type ");
        } 
        else{
            printf(" Not Bootable, Type ");
        }
        printf_hex(mbr.primary_partition[i].partition_id);

        printf("\n=== Using new FAT32 class ===\n");
        FAT32 fs(hd, mbr.primary_partition[i].start_lba);
        
        printf("Mounting... ");
        if(fs.mount() != FSError::SUCCESS){
            printf("Mount unsuccessful\n ");
            return;
        }
        printf("Mounted ");
        printf((char*)fs.get_fs_name());
        printf(": ");
        printf_hex(i);
        printf("\n");
        
        fs.print_list_directory(printf, "/");
        
        FileHandle file1;
        FSError err = fs.open("/FILE2.TXT", file1);
        if(err != FSError::SUCCESS){
            printf("Failed to open ");
            return;
        }
        uint8_t buf[512];
        uint32_t bytes_read{};
        err = fs.read(file1, buf, 512, bytes_read);
        if(err != FSError::SUCCESS){
            printf("Failed to read ");
            return;
        }
        printf("0x");
        printf_hex(bytes_read);
        printf(" bytes read\n     ");
        buf[bytes_read+1] = '\0';
        printf((char*)buf);
        printf("\n");
    }

}

MSDOSPartitionTable::MSDOSPartitionTable(ATA* hd){
    MasterBootRecord m_mbr;
    hd->read_28(0, (uint8_t*)&m_mbr, sizeof(MasterBootRecord));
    if(m_mbr.magic_number != 0xAA55){
        printf("Illegal MBR!");
        return;
    }
}

MSDOSERR MSDOSPartitionTable::get_partition_offset(uint32_t& offset, int partition_number){
    if(m_mbr.primary_partition[partition_number].partition_id == 0x00)
        return MSDOSERR::PARTITION_DOES_NOT_EXIST;
    
    offset = m_mbr.primary_partition[partition_number].start_lba;
    return MSDOSERR::SUCCESS;
}