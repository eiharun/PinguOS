#include <gtest/gtest.h>

#include <filesystem/fat.h>
#include <allocator.h>
#include "mock_ata.hpp"

using namespace filesystem;

// Write LE little-endian helpers
static void w16(uint8_t* p, uint16_t v) {
    p[0] = v & 0xFF;
    p[1] = (v >> 8) & 0xFF;
}
static void w32(uint8_t* p, uint32_t v) {
    p[0] = v & 0xFF;
    p[1] = (v >> 8) & 0xFF;
    p[2] = (v >> 16) & 0xFF;
    p[3] = (v >> 24) & 0xFF;
}

TEST(FAT32, OpenRootPrintAndClose)
{
    // -------------------------------------------------------
    // 1. Build mock ATA
    // -------------------------------------------------------
    MockATA disk;

    uint8_t* sector0 = disk.m_disk.data();

    // -------------------------------------------------------
    // 2. Build a minimal FAT32 BPB in sector 0
    // -------------------------------------------------------
    sector0[0] = 0xEB; sector0[1] = 0x58; sector0[2] = 0x90;

    // Bytes per sector = 512
    w16(&sector0[11], 512);

    // Sectors per cluster = 1
    sector0[13] = 1;

    // Reserved sectors = 32
    w16(&sector0[14], 32);

    // Number of FATs = 1
    sector0[16] = 1;

    // Root entry count = 0 (FAT32)
    w16(&sector0[17], 0);

    // Total 16-bit sectors = 0 => use 32-bit below
    w16(&sector0[19], 0);

    // Media descriptor
    sector0[21] = 0xF8;

    // FAT32 FAT size = 1 sector
    w32(&sector0[36], 1);

    // Root cluster = 2
    w32(&sector0[44], 2);

    // Total sectors = 128 (your disk size)
    w32(&sector0[32], 128);

    // -------------------------------------------------------
    // 3. Write the FAT
    // FAT starts at sector 32
    // -------------------------------------------------------
    uint32_t fat_sector = 32;
    uint8_t* fat = &disk.m_disk[fat_sector * MockATA::SECTOR_SIZE];

    // Mark cluster 2 as end-of-chain
    w32(&fat[2 * 4], 0x0FFFFFFF);

    // -------------------------------------------------------
    // 4. Build root directory at cluster 2
    // cluster 2 == sector 32 + FAT sectors (1) + (cluster-2)*SPC
    //
    // Reserved 32
    // FAT table 1
    // Therefore cluster 2 = sector 33
    // -------------------------------------------------------
    uint32_t root_sector = 33;
    uint8_t* root = &disk.m_disk[root_sector * 512];

    // Create a short-name entry: "FILE    TXT"
    memcpy(root, "FILE    TXT", 11);
    root[11] = 0x20;        // archive attribute

    // starting cluster = 2
    w16(&root[0x1A], 2);    // low
    w16(&root[0x14], 0);    // high
    w32(&root[0x1C], 0);    // size

    root[32] = 0x00; // end-of-directory entry

    // -------------------------------------------------------
    // 5. Mount FAT32
    // -------------------------------------------------------
    FAT32 fs(&disk, /*partition_offset=*/0);

    ASSERT_EQ(fs.mount(), FSError::SUCCESS);

    // -------------------------------------------------------
    // 6. Open directory "/"
    // -------------------------------------------------------
    DirectoryIterator* it = nullptr;
    ASSERT_EQ(fs.open_directory("/", it), FSError::SUCCESS);
    ASSERT_NE(it, nullptr);

    // -------------------------------------------------------
    // 7. Iterate directory entries
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

    ASSERT_TRUE(saw_file);

    // -------------------------------------------------------
    // 8. Clean up
    // -------------------------------------------------------
    fs.close_directory(it);
}
