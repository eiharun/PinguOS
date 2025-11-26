#include <cstddef>
#include <cstdlib>
#include <gtest/gtest.h>

#include <filesystem/fat.h>
#include <allocator.h>
#include <memory>
#include "filesystem/filesystem.h"
#include "mock_ata.hpp"

using namespace filesystem;

#define ASSERT_SUCCESS(item) ASSERT_EQ(item, FSError::SUCCESS)
#define ASSERT_FAIL(item) ASSERT_NE(item, FSError::SUCCESS)

namespace filesystem{
std::ostream& operator<<(std::ostream& os, const FSError& status) {
    switch (status) {
        case FSError::SUCCESS: {
            os << "FSError::SUCCESS\n";
            break;
        }
        case FSError::NOT_FOUND: {
            os << "FSError:NOT_FOUND:\n";
            break;
        }
        case FSError::INVALID_PATH: {
            os << "FSError::INVALID_PATH\n";
            break;
        }
        case FSError::NOT_A_DIRECTORY: {
            os << "FSError::NOT_A_DIRECTORY\n";
            break;
        }
        case FSError::NOT_A_FILE: {
            os << "FSError::NOT_A_FILE\n";
            break;
        }
        case FSError::END_OF_DIRECTORY: {
            os << "FSError::END_OF_DIRECTORY\n";
            break;
        }
        case FSError::READ_ERROR: {
            os << "FSError::READ_ERROR\n";
            break;
        }
        case FSError::DISK_ERROR: {
            os << "FSError::DISK_ERROR\n";
            break;
        }
        case FSError::INVALID_FILESYSTEM: {
            os << "FSError::INVALID_FILESYSTEM\n";
            break;
        }
        case FSError::BUFFER_TOO_SMALL: {
            os << "FSError::BUFFER_TOO_SMALL\n";
            break;
        }
        case FSError::OUT_OF_MEMORY: {
            os << "FSError::OUT_OF_MEMORY\n";
            break;
        }
        case FSError::ALREADY_EXISTS: {
            os << "FSError::ALREADY_EXISTS\n";
            break;
        }
        case FSError::DISK_FULL: {
            os << "FSError:DISK_FULL:\n";
            break;
        }
        case FSError::END_OF_CHAIN: {
            os << "FSError::END_OF_CHAIN\n";
            break;
        }
        case FSError::NOT_IMPLEMENTED: {
            os << "FSError::NOT_IMPLEMENTED\n";
            break;
        }
        default:
            os << "Unknown Status Value (" << static_cast<int>(status) << ")";
            break;
    }
    return os;
}
}

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
        fat[2] = 0x0FFFFFFF;

        // -------------------------------------------------------
        // Build root directory at cluster 2
        // cluster 2 == sector 32 + FAT sectors (1) + (cluster-2)*SPC
        //
        // Reserved 32
        // FAT table 1
        // Therefore cluster 2 = sector 33
        // -------------------------------------------------------
        uint32_t data_root_sector = fat_sector + bpb.fat_copies * bpb.table_size;

        // Create a short-name entry: "FILE    TXT"
        DirectoryEntryFat32 f1;
        char file_data[256] {"This is a mock file in MOCK ATA driver. TEST ME!!!\n"};
        uint8_t name[12] = "FILE    TXT";   // PAD back name with spaces
        for(int i=0; i<11;++i){
            f1.name[i] = name[i];
        }
        f1.attributes = 0x20;       // archive attribute

        // starting cluster = 3
        f1.first_cluster_hi = 0x0000;
        f1.first_cluster_lo = 0x0003;
        fat[3] = 0x0FFFFFFF;
        f1.size = std::strlen(file_data);
        DirectoryEntryFat32 end;
        end.name[0] = 0x00; // end-of-directory entry
        DirectoryEntryFat32 entries[2] {f1, end};
        disk.write_28(data_root_sector, (uint8_t*)entries, 2*sizeof(DirectoryEntryFat32));
        // Fill file contents
        uint32_t data_file_sector = data_root_sector + (3-2)*bpb.sectors_per_cluster;
        disk.write_28(data_file_sector, (uint8_t*)file_data, std::strlen(file_data));
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

        // FAT32 FAT size = 16 sector
        bpb.table_size = 16;
        bpb.fat_sector_count = (uint16_t)(bpb.table_size);

        // Root cluster = 2
        bpb.root_cluster = 2;

        // Total sectors = 128 (your disk size)
        bpb.total_sectors = sector_count;

        return bpb;
    }

};

#define GA_PARTITION_OFFSET 0
class FAT32_GA: public testing::Test {
protected:
    FAT32_GA() = default;
    static void SetUpTestSuite() {
        std::cout << "Initializing Mock ATA Disk\n";
        disk = std::make_unique<MockATA>(DiskSetup::make_disk(1024));
        bpb = DiskSetup::build_bpb(1024, disk->SECTOR_SIZE);
        fs = std::make_unique<FAT32>(disk.get(), GA_PARTITION_OFFSET);
        ASSERT_EQ(fs->mount(), FSError::SUCCESS);
    };
    static uint32_t get_next_cluster(uint32_t cluster){
        uint32_t fat_start = GA_PARTITION_OFFSET+bpb.reserved_sectors;
        uint32_t fat_sector = cluster / (bpb.bytes_per_sector / 4);
        uint32_t fat_offset = cluster % (bpb.bytes_per_sector / 4);
        uint8_t sector_buf[MOCK_SECTOR_SIZE]{};
        disk->read_28(fat_start+fat_sector, sector_buf, bpb.bytes_per_sector);
        return ((uint32_t*)sector_buf)[fat_offset] & 0x0FFFFFFF;
    };
    static size_t num_clusters(uint32_t start_cluster, std::vector<uint32_t>* clusters = nullptr){
        size_t count{};
        uint32_t cluster = start_cluster;
        uint32_t fat_start = GA_PARTITION_OFFSET+bpb.reserved_sectors;
        while((cluster < 0x0FFFF8) && (cluster != 0)){
            uint32_t fat_sector = cluster / (bpb.bytes_per_sector / 4);
            uint32_t fat_offset = cluster % (bpb.bytes_per_sector / 4);
            uint8_t sector_buf[MOCK_SECTOR_SIZE]{};
            disk->read_28(fat_start+fat_sector, sector_buf, bpb.bytes_per_sector);
            cluster = ((uint32_t*)sector_buf)[fat_offset] & 0x0FFFFFFF;
            if(cluster !=0 ){
                count ++;
                if(clusters) clusters->push_back(cluster);
            }
        }
        return count;
    };
    static bool check_fat_integrity(FileHandle& handle){
        // Checks FAT chain to ensure every chain has a EOC and no NULL
        uint32_t cluster = handle.start_cluster;
        uint32_t fat_start = GA_PARTITION_OFFSET+bpb.reserved_sectors;
        while((cluster < 0x0FFFF8)){
            uint32_t fat_sector = cluster / (bpb.bytes_per_sector / 4);
            uint32_t fat_offset = cluster % (bpb.bytes_per_sector / 4);
            uint8_t sector_buf[MOCK_SECTOR_SIZE]{};
            disk->read_28(fat_start+fat_sector, sector_buf, bpb.bytes_per_sector);
            cluster = ((uint32_t*)sector_buf)[fat_offset] & 0x0FFFFFFF;
            if(cluster == 0 ){
                return false;
            }
        }
        return true;
    };

    static BiosParameterBlock32 bpb;
    static std::unique_ptr<MockATA> disk;
    static std::unique_ptr<FAT32> fs;
};
BiosParameterBlock32 FAT32_GA::bpb;
std::unique_ptr<MockATA> FAT32_GA::disk;
std::unique_ptr<FAT32> FAT32_GA::fs;


TEST_F(FAT32_GA, OpenRootPrintAndClose)
{
    // -------------------------------------------------------
    // Open directory "/"
    // -------------------------------------------------------
    DirectoryIterator* it = nullptr;
    ASSERT_EQ(fs->open_directory("/", it), FSError::SUCCESS);
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
    fs->close_directory(it);
}

TEST_F(FAT32_GA, TestMakeFile){
    // make_file must be successful
    ASSERT_SUCCESS(fs->make_file("/", "NEW_FILE.TXT"));
    DirectoryIterator* it = nullptr;
    ASSERT_EQ(fs->open_directory("/", it), FSError::SUCCESS);
    ASSERT_NE(it, nullptr);
    
    FileEntry e;
    bool saw_file = false;

    while (it->next(e) == FSError::SUCCESS)
    {
        printf("Name: %s  FirstCluster: %u  Attr: 0x%X\n",
               e.name,
               e.first_cluster,
               e.attributes);

        if (strncmp(e.name, "NEW_FILE.TXT", 8) == 0)
            saw_file = true;
    }
    EXPECT_TRUE(saw_file);

    fs->close_directory(it);
}

TEST_F(FAT32_GA, TestMakeFileBadPath){
    ASSERT_FAIL(fs->make_file("/incorrect", "NEW_FILE.TXT"));
    ASSERT_FAIL(fs->make_file("incorrect", "NEW_FILE.TXT"));
}

TEST_F(FAT32_GA, TestMakeFileLongName){
    ASSERT_SUCCESS(fs->make_file("/", "LONGFILENAME.TXT"));
    DirectoryIterator* it = nullptr;
    ASSERT_EQ(fs->open_directory("/", it), FSError::SUCCESS);
    ASSERT_NE(it, nullptr);
    
    FileEntry e;
    bool saw_file = false;

    while (it->next(e) == FSError::SUCCESS)
    {
        printf("Name: %s  FirstCluster: %u  Attr: 0x%X\n",
               e.name,
               e.first_cluster,
               e.attributes);

        if (strncmp(e.name, "LONGFILE.TXT", 11) == 0)
            saw_file = true;
    }
    EXPECT_TRUE(saw_file);

    fs->close_directory(it);
}

TEST_F(FAT32_GA, TestMakeFileDuplicateFile){
    ASSERT_EQ(fs->make_file("/", "FILE.TXT"), FSError::ALREADY_EXISTS);
}

TEST_F(FAT32_GA, TestOpenAndReadFile){
    FileHandle handle;
    ASSERT_SUCCESS(fs->open("/FILE.TXT", handle));
    EXPECT_TRUE(handle.valid);
    uint8_t buf[256]{};
    uint32_t bytes_read{};
    EXPECT_EQ(handle.size, 51);
    ASSERT_SUCCESS(fs->read(handle,buf, handle.size, bytes_read));
    printf((char*)buf);
    EXPECT_EQ(bytes_read, 51);
}

TEST_F(FAT32_GA, TestReadInvalidFile){
    FileHandle handle;
    uint8_t buf[256]{};
    uint32_t bytes_read{};
    ASSERT_FAIL(fs->read(handle,buf, handle.size, bytes_read));
}

TEST_F(FAT32_GA, TestWriteLongFile){
    FileHandle handle;
    ASSERT_SUCCESS(fs->open("/LONGFILE.TXT", handle));
    EXPECT_TRUE(handle.valid);
    uint32_t old_size = handle.size;

    constexpr size_t BUF_SIZE = 512*18; // 3 clusters long
    uint8_t buf[BUF_SIZE]{""};
    for(size_t i=0; i<BUF_SIZE-100; i++){
        buf[i] = (i % 126) + 32;
    }
    ASSERT_EQ(std::strlen((char*)buf), BUF_SIZE-100);
    ASSERT_SUCCESS(fs->write(handle, buf, BUF_SIZE-100, filesystem::Filesystem::WRITE));
    EXPECT_GT(handle.size, old_size);

    // Reset handle position 
    // (would normally do this with seek(), but since it hasn't been tested yet, 
    // this is enough to point to the start of the file)
    handle.position = 0;
    handle.cluster = handle.start_cluster;

    uint8_t read_buf[BUF_SIZE]{};
    size_t bytes_read{};
    ASSERT_SUCCESS(fs->read(handle, read_buf, BUF_SIZE-100, bytes_read));
    EXPECT_EQ(std::strlen((char*)read_buf), BUF_SIZE-100);
    ASSERT_NE(read_buf[0], '\0');
    for(int i=0; i<BUF_SIZE-100; ++i){
        EXPECT_EQ(read_buf[i], buf[i]);
    }
    fs->close(handle);
    EXPECT_FALSE(handle.valid);
}

TEST_F(FAT32_GA, TestSeek){
    // Verify file contents 
    FileHandle handle;
    ASSERT_SUCCESS(fs->open("/LONGFILE.TXT", handle));
    EXPECT_TRUE(handle.valid);
    handle.position = 0;
    handle.cluster = handle.start_cluster;
    // Contents should be 32 -> 158 ascii
    // Read first 126 bytes
    const size_t BUF_SIZE = 1024;
    uint8_t read_buf[BUF_SIZE]{};
    uint32_t bytes_read{};
    ASSERT_SUCCESS(fs->read(handle, read_buf, 126, bytes_read));
    EXPECT_EQ(bytes_read, 126);
    EXPECT_EQ(std::strlen((char*)read_buf), 126);
    printf("+0:\t");
    printf((char*)read_buf);
    printf("\n");
    for(int i=0; i<bytes_read; ++i){
        EXPECT_EQ(read_buf[i], (i % 126) + 32);
    }
    // Start small +3
    ASSERT_SUCCESS(fs->seek(handle, 3));
    EXPECT_EQ(handle.position, 3);
    EXPECT_EQ(handle.cluster, handle.start_cluster);
    std::memset(read_buf, 0, BUF_SIZE);
    ASSERT_SUCCESS(fs->read(handle, read_buf, 126, bytes_read));
    EXPECT_EQ(bytes_read, 126);
    printf("+3:\t");
    printf((char*)read_buf);
    printf("\n");
    for(int i=0; i<bytes_read; ++i){
        EXPECT_EQ(read_buf[i], ((i+3) % 126) + 32);
    }
    // Cross over into next cluster
    constexpr int seek_pos_1 = (126*5) + 50;
    ASSERT_SUCCESS(fs->seek(handle, seek_pos_1));
    std::memset(read_buf, 0, BUF_SIZE);
    ASSERT_SUCCESS(fs->read(handle, read_buf, 126, bytes_read));
    EXPECT_EQ(bytes_read, 126);
    printf("+%d:\t",seek_pos_1);
    printf((char*)read_buf);
    printf("\n");
    for(int i=0; i<bytes_read; ++i){
        EXPECT_EQ(read_buf[i], ((i+seek_pos_1) % 126) + 32);
    }
    fs->close(handle);
}

TEST_F(FAT32_GA, TestWriteLongFileOverWriteShort){
    // Test to make sure cluster was freed; get_next_cluster() = 0
    FileHandle handle;
    ASSERT_SUCCESS(fs->open("/LONGFILE.TXT", handle));
    std::vector<uint32_t> clusters;
    size_t cluster_count_before = num_clusters(handle.start_cluster, &clusters);
    EXPECT_TRUE(handle.valid);

    const int BUF_SIZE = 13;
    uint8_t buf[BUF_SIZE] {"Hello World\n"};
    ASSERT_SUCCESS(fs->write(handle, buf, BUF_SIZE, filesystem::Filesystem::WRITE));
    size_t cluster_count_after = num_clusters(handle.start_cluster);
    EXPECT_EQ(cluster_count_after, 1);
    EXPECT_TRUE(check_fat_integrity(handle));
    EXPECT_LT(cluster_count_after, cluster_count_before);
    int cluster_diff = cluster_count_before - cluster_count_after;
    // Last 2 clusters should be 0
    for(int i=1; i<cluster_diff; ++i){
        EXPECT_EQ(get_next_cluster(clusters[i]), 0);
    }
    
    uint8_t read_buf[BUF_SIZE]{};
    uint32_t bytes_read{};
    ASSERT_SUCCESS(fs->read(handle, read_buf, BUF_SIZE, bytes_read));
    printf((char*)read_buf);
    ASSERT_EQ(bytes_read, BUF_SIZE);
    for(int i=0; i<BUF_SIZE; ++i){
        EXPECT_EQ(buf[i], read_buf[i]);
    }

    fs->close(handle);
}

TEST_F(FAT32_GA, TestWriteAppendToLongFileEnd){
    GTEST_SKIP(); 
}

TEST_F(FAT32_GA, TestWriteAppendToLongFileMiddle){
    GTEST_SKIP();
}

/*
Tests list
FAT32::open - use correct path, use nonexistent path x2
FAT32::write - ensure correct write, use size 0, test write and append mode. Write with multiple clusters
FAT32::read - ensure correct read, verify bytes read, give incorrect/invalid file handle. Read with multiple clusters
FAT32::seek - ensure correct seek position across multiple clusters
FAT32::close - ensure file handle is invalid
FAT32::make_file - ensure new dir entry is created, give a non root path(should be error), give a file name longer than 8 char
FAT32::delete_file - ensure dir entry is marked as deleted
FAT32::make_directory/delete_directory - same as file
FAT32::open_directory - ensure iterator is not nullptr, use incorrect/nonexistent path
FAT32::close_directory - use count free / count malloc in host_alloc.h/cpp, try with nonalloced iterator/nullptr
FAT32::stat - use incorrect path/non root path
FAT32::exists - use files and dirs, include files that have been deleted

FAT32DirectoryIterator - create multiple directories and files and build file that is spread across clusters
*/