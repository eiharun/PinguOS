#pragma once
#include <common/types.h>
#include <drivers/disk.h>
#include <filesystem/filesystem.h>

using namespace common;

namespace filesystem {

struct BiosParameterBlock32{
    uint8_t jump[3];
    uint8_t soft_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t total_sectors;
    uint8_t media_type;
    uint16_t fat_sector_count;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sectors;
    uint32_t total_sector_count;

    //Fat32
    uint32_t table_size;
    uint16_t ext_flags;
    uint16_t fat_version;
    uint32_t root_cluster;
    uint16_t fat_info;
    uint16_t backup_sector;
    uint8_t reserved0[12];
    uint8_t drive_num;
    uint8_t reserved_1;
    uint8_t boot_sig;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fat_type_label[8];
}__attribute__((packed));

#define ENTRY_END 0x00
#define ENTRY_DELETED 0xE5
#define IS_END_OF_CHAIN(cluster) cluster>=0x0FFFF8
#define NAME_SIZE_8_3 11

struct DirectoryEntryFat32{
    uint8_t name[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t c_tenth_time;
    uint16_t c_time;
    uint16_t c_date;
    uint16_t a_time;
    uint16_t first_cluster_hi;
    uint16_t w_time;
    uint16_t w_date;
    uint16_t first_cluster_lo;
    uint32_t size;
}__attribute__((packed));

struct LongFileName32{
    uint8_t order;
    uint16_t name1[5];
    uint8_t attribute;
    uint8_t type;
    uint8_t checksum;
    uint16_t name2[6];
    uint16_t reserved_0;
    uint16_t name3[2];
}__attribute__((packed));


class FAT32DirectoryIterator: public DirectoryIterator{
public:
    FAT32DirectoryIterator(drivers::Disk* disk, BiosParameterBlock32* bpb, uint32_t partition_offset, uint32_t start_cluster);
    void load_cluster();
    void write_back_cluster();
    FSError next(FileEntry& file) override;
    FSError load_next_cluster();
    void delete_dir_entry();
    void convert_entry(DirectoryEntryFat32* src, FileEntry& dst);
    void reset() override;
private:
    drivers::Disk* m_disk;
    BiosParameterBlock32* m_bpb;
    uint32_t m_partition_offset;
    uint8_t m_cluster_buffer[512 * 8]; 
    uint32_t m_entry_index;             
    uint32_t m_entries_per_cluster;
    uint32_t m_current_cluster;
    uint32_t m_start_cluster;
    bool m_finished;
};

class FAT32: public Filesystem{
public:
    FAT32(drivers::Disk* disk, uint32_t partition_offset);
    ~FAT32(); 

    FSError mount() override;
    void unmount() override;
    static bool detect(uint8_t* boot_sector);

    FSError open(const char* path, FileHandle& handle) override;
    FSError write(FileHandle& handle, uint8_t* buffer, size_t size, WRITE_MODE mode) override;
    FSError read(FileHandle& handle, uint8_t* buffer, size_t size, uint32_t& bytes_read) override;
    FSError seek(FileHandle& handle, uint32_t position) override;
    void close(FileHandle& handle) override;

    FSError make_file(const char* path, const char* filename) override;
    FSError delete_file(FileHandle& handle) override;
    FSError make_directory(const char* path, const char* dirname) override;
    FSError delete_directory(const char* path) override;

    FSError open_directory(const char* path, DirectoryIterator*& iterator) override;
    void close_directory(DirectoryIterator* iterator) override;
    FSError stat(const char* path, FileEntry& file) override;
    bool exists(const char* path) override;
    const char* get_fs_name() const override;
    FSError print_list_directory(void(*printf)(char*), const char* path);

private:
    uint32_t get_next_cluster(uint32_t cluster);
    void link_next_cluster(uint32_t current_cluster, uint32_t next_cluster);
    FSError read_cluster(uint32_t cluster, uint8_t* buffer, uint32_t offset, uint32_t size);
    FSError write_cluster(uint32_t cluster, uint8_t* buffer, uint32_t offset, uint32_t size);
    FSError zero_cluster(uint32_t cluster);
    uint32_t allocate_cluster();
    FSError create_directory_entry(FileEntry& new_entry);

    drivers::Disk* m_disk;
    BiosParameterBlock32 m_bpb;
    uint32_t m_fat_start;
    uint32_t m_data_start;
    uint8_t m_sector_buf[512];
};


}