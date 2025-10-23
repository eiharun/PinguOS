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

    const uint32_t SECTOR_SIZE = bpb.bytes_per_sector;
    const uint32_t cluster_size_b = bpb.sectors_per_cluster * SECTOR_SIZE;
    uint8_t root_cluster[cluster_size_b];

    for(int i=0; i<bpb.sectors_per_cluster; ++i){
        hd->read_28(root_start+i, root_cluster+(SECTOR_SIZE*i), SECTOR_SIZE);
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
        
        int32_t SIZE = dir[i].size;
        int32_t next_file_cluster = file_cluster;
        uint8_t buf[SECTOR_SIZE+1];
        while(SIZE > 0){
            uint32_t file_sector = data_start + bpb.sectors_per_cluster * (next_file_cluster-2);
            uint32_t sector_offset{};
            
            for(; SIZE>0; SIZE -= SECTOR_SIZE){
                hd->read_28(file_sector+sector_offset, buf, SECTOR_SIZE);
                buf[SIZE > SECTOR_SIZE ? SECTOR_SIZE : SIZE] = '\0';
                printf("    ");
                printf((char*)buf);
                if(++sector_offset > bpb.sectors_per_cluster-1){
                    break;
                }
            }
            uint8_t fatbuf[SECTOR_SIZE+1];
            uint32_t fat_sector_curr_cluster = next_file_cluster / (SECTOR_SIZE/sizeof(uint32_t));
            hd->read_28(fat_start + fat_sector_curr_cluster, fatbuf, SECTOR_SIZE);
            uint32_t fat_offset_in_sector_curr_cluster = next_file_cluster % (SECTOR_SIZE/sizeof(uint32_t));
            next_file_cluster = ((uint32_t*)&fatbuf)[fat_offset_in_sector_curr_cluster] & 0x0FFFFFFF;
        }
    }
    printf("\n\n");

}
