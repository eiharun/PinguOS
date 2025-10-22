#include <filesystem/fat.h>

// using namespace drivers;
using namespace filesystem;

void printf(char*);
void printf_hex(uint8_t);
void printf_hex16(uint16_t);
void printf_hex32(uint32_t);

void filesystem::read_bios_block(drivers::ATA* hd, uint32_t partition_offset){
    BiosParameterBlock32 bpb;
    hd->read_28(partition_offset, (uint8_t*)&bpb, sizeof(BiosParameterBlock32));
   
    uint32_t fat_start = partition_offset + bpb.reserved_sectors;
    uint32_t fat_size = bpb.table_size;

    uint32_t data_start = fat_start + fat_size*bpb.fat_copies;

    uint32_t root_start = data_start + bpb.sectors_per_cluster*(bpb.root_cluster-2);

    const uint32_t sector_size = bpb.bytes_per_sector;
    const uint32_t cluster_size_b = bpb.sectors_per_cluster * sector_size;
    uint8_t root_cluster[cluster_size_b];

    for(int i=0; i<bpb.sectors_per_cluster; ++i){
        hd->read_28(root_start+i, root_cluster+(sector_size*i), sector_size);
    }

    DirectoryEntryFat32* dir = (DirectoryEntryFat32*)root_cluster;
    uint32_t num_entries = cluster_size_b / sizeof(DirectoryEntryFat32);
    // DirectoryEntryFat32 dir[16];
    // hd->read_28(root_start, (uint8_t*)&dir[0], sizeof(DirectoryEntryFat32)*16);
    printf("\n\n");
    for(int i=0 ; i<16; ++i){
        if(dir[i].name[0] == 0x00){
            break;
        }
        if((dir[i].attributes & 0x0F) == 0x0F){
            continue;
        }
        char* foo = "        \n";
        for(int j = 0; j<8; ++j){
            foo[j] = dir[i].name[j];
        }
        printf(foo);

        if((dir[i].attributes & 0x10) == 0x10){
            continue; //DIRECTORY
        }

        uint32_t file_cluster = ((uint32_t)dir[i].first_cluster_hi) << 16 | (uint32_t)dir[i].first_cluster_lo;
        uint32_t file_sector = data_start + bpb.sectors_per_cluster * (file_cluster-2);
        uint8_t buf[512];
        hd->read_28(file_sector, buf, 512);
        buf[dir[i].size] = '\0';
        printf("    ");
        printf((char*)buf);
    }
    printf("\n\n");

}
