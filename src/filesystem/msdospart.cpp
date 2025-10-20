#include "common/types.h"
#include <filesystem/msdospart.h>

using namespace filesystem;

void printf(char*);
void printf_hex(uint8_t);
void MSDOSPartitionTable::read_partitions(ATA* hd){
    MasterBootRecord mbr;
    hd->read_28(0, (uint8_t*)&mbr, sizeof(MasterBootRecord));

    printf("MBR: ");
    for(int i=0x1BE; i<=0x01FF; i++){
        printf_hex(((uint8_t*)&mbr)[i]);
        printf(" ");
    }
    printf("\n");
    if(mbr.magic_number != 0xAA55){
        printf("Illegal MBR!");
        return;
    }
}
