#include "common/types.h"
#include <filesystem/msdospart.h>
#include <filesystem/fat.h>

using namespace filesystem;

void printf(char*);
void printf_hex(uint8_t);
void MSDOSPartitionTable::read_partitions(ATA* hd){
    MasterBootRecord mbr;
    hd->read_28(0, (uint8_t*)&mbr, sizeof(MasterBootRecord));
    // printf("MBR: ");
    // for(int i=0x1BE; i<=0x01FF; i++){
    //     printf_hex(((uint8_t*)&mbr)[i]);
    //     printf(" ");
    // }
    // printf("\n");
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

        read_bios_block(hd, mbr.primary_partition[i].start_lba);
    }

}
