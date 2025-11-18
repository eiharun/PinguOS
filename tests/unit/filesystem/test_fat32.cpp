#include <gtest/gtest.h>

#include <filesystem/fat.h>
#include <allocator.h>
#include <memory>
#include "mock_ata.hpp"

using namespace filesystem;

struct DiskSetup{
    static MockATA make_disk(size_t sector_count){
        MockATA disk(sector_count);
        BiosParameterBlock32 bpb = build_bpb(sector_count, disk.SECTOR_SIZE);
        disk.write_28(0, (uint8_t*)&bpb, sizeof(BiosParameterBlock32));

        // -------------------------------------------------------
        // Write the FAT
        // FAT starts at partition_offset + bpb.reserved_sectors
        // -------------------------------------------------------
        uint32_t fat_sector = bpb.reserved_sectors;
        uint32_t* fat = (uint32_t*)&disk.m_data[fat_sector * disk.SECTOR_SIZE];

        // Mark cluster 2 as end-of-chain = root cluster
        fat[2 * 4] = 0x0FFFFFFF;

        // -------------------------------------------------------
        // Build root directory at cluster 2
        // cluster 2 == sector 32 + FAT sectors (1) + (cluster-2)*SPC
        //
        // Reserved 32
        // FAT table 1
        // Therefore cluster 2 = sector 33
        // -------------------------------------------------------
        uint32_t root_sector = fat_sector + bpb.fat_copies * bpb.table_size;

        // Create a short-name entry: "FILE    TXT"
        DirectoryEntryFat32 f1;
        uint8_t name[9] = "FILE    ";   // PAD back name with spaces
        uint8_t ext[4] = "TXT";         // PAD front ext with spaces
        for(int i=0; i<8;++i){
            f1.name[i] = name[i];
        }
        for(int i=0; i<3;++i){
            f1.ext[i] = ext[i];
        }
        f1.attributes = 0x20;       // archive attribute
        
        // starting cluster = 3
        f1.first_cluster_hi = 0x0000;
        f1.first_cluster_lo = 0x0003;
        f1.size = 0;
        DirectoryEntryFat32 end;
        end.name[0] = 0x00; // end-of-directory entry
        DirectoryEntryFat32 entries[2] {f1, end};
        disk.write_28(root_sector, (uint8_t*)entries, 2*sizeof(DirectoryEntryFat32));
        return disk;
    }

    static BiosParameterBlock32 build_bpb(size_t sector_count, size_t sector_size){
        // Build a minimal FAT32 BPB in sector 0
        BiosParameterBlock32 bpb;
        bpb.jump[0] = 0xEB; 
        bpb.jump[1] = 0x58; 
        bpb.jump[2] = 0x90;
        uint8_t soft_name[8] = "GTEST";
        for(int i=0; i<8; ++i){
            bpb.soft_name[i] = soft_name[i];
        }

        // Bytes per sector = 512
        bpb.bytes_per_sector = sector_size;

        // Sectors per cluster = 1
        bpb.sectors_per_cluster = 8;

        // Reserved sectors = 32
        bpb.reserved_sectors = 32;

        // Number of FATs = 1
        bpb.fat_copies = 1;

        // Root entry count = 0 (FAT32)
        bpb.root_dir_entries = 0;

        // Total 16-bit sectors = 0 => use 32-bit below
        bpb.total_sectors = 0;

        // Media descriptor
        bpb.media_type = 0xF8;

        // FAT32 FAT size = 1 sector
        bpb.table_size = 1;
        bpb.fat_sector_count = (uint16_t)(bpb.table_size);

        // Root cluster = 2
        bpb.root_cluster = 2;

        // Total sectors = 128 (your disk size)
        bpb.total_sectors = sector_count;

        return bpb;
    }

    static bool add_file(){}
    static bool add_dir(){}
};

class FAT32_GA: public testing::Test {
protected:
    FAT32_GA() = default;
    static void SetUpTestSuite() {
        std::cout << "Initializing Mock ATA Disk\n";
        disk = std::make_unique<MockATA>(DiskSetup::make_disk(128));
    };

    static std::unique_ptr<MockATA> disk;
};
std::unique_ptr<MockATA> FAT32_GA::disk;

// TEST_F(FAT32_GA, TestMockATAFixture){

// }

TEST_F(FAT32_GA, OpenRootPrintAndClose)
{
    // -------------------------------------------------------
    // Mount FAT32
    // -------------------------------------------------------
    FAT32 fs(disk.get(), /*partition_offset=*/0);

    ASSERT_EQ(fs.mount(), FSError::SUCCESS);

    // -------------------------------------------------------
    // Open directory "/"
    // -------------------------------------------------------
    DirectoryIterator* it = nullptr;
    ASSERT_EQ(fs.open_directory("/", it), FSError::SUCCESS);
    ASSERT_NE(it, nullptr);

    // -------------------------------------------------------
    // Iterate directory entries
    // -------------------------------------------------------
    FileEntry e;
    bool saw_file = false;

    while (it->next(e) == FSError::SUCCESS)
    {
        printf("Name: %s  FirstCluster: %u  Attr: 0x%X\n",
               e.name,
               e.first_cluster,
               e.attributes);

        if (strncmp(e.name, "FILE.TXT", 8) == 0)
            saw_file = true;
    }

    EXPECT_TRUE(saw_file);

    // -------------------------------------------------------
    // Clean up
    // -------------------------------------------------------
    fs.close_directory(it);
}
