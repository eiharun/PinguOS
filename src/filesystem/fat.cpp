#include "filesystem/filesystem.h"
#include <filesystem/fat.h>
#include <memory_management.h>
#include <common/path_utils.hpp>

using namespace filesystem;

void printf(char*);
void printf_hex(uint8_t);
void printf_hex16(uint16_t);
void printf_hex32(uint32_t);

FAT32DirectoryIterator::FAT32DirectoryIterator(drivers::ATA* disk, BiosParameterBlock32* bpb, uint32_t partition_offset, uint32_t start_cluster)
: m_disk(disk), m_bpb(bpb), m_partition_offset(partition_offset), m_entry_index(0), m_start_cluster(start_cluster)
{
    m_current_cluster = start_cluster;
    m_finished = false;
    m_entries_per_cluster = (bpb->sectors_per_cluster * bpb->bytes_per_sector) / sizeof(DirectoryEntryFat32);
    load_cluster();
}

void FAT32DirectoryIterator::load_cluster(){
    if(m_current_cluster >= 0x0FFFFFF8){
        m_finished = true;
        return;
    }

    uint32_t fat_start = m_partition_offset + m_bpb->reserved_sectors;
    uint32_t fat_size = m_bpb->table_size;

    uint32_t data_start = fat_start + fat_size*m_bpb->fat_copies;

    uint32_t cluster_sector = data_start + m_bpb->sectors_per_cluster*(m_current_cluster-2);

    for(int i=0; i<m_bpb->sectors_per_cluster; ++i){
        m_disk->read_28(cluster_sector+i, m_cluster_buffer+(m_bpb->bytes_per_sector*i), m_bpb->bytes_per_sector);
    }

    m_entry_index = 0;
}

FSError FAT32DirectoryIterator::load_next_cluster(){
    uint32_t fat_start = m_partition_offset + m_bpb->reserved_sectors;
    uint32_t fat_sector = m_current_cluster / (m_bpb->bytes_per_sector / 4);
    uint32_t fat_offset = m_current_cluster % (m_bpb->bytes_per_sector / 4);

    uint8_t fat_buf[512];
    m_disk->read_28(fat_start+fat_sector, fat_buf, m_bpb->bytes_per_sector);
    uint32_t next_cluster = ((uint32_t*)fat_buf)[fat_offset] & 0x0FFFFFFF;

    if(next_cluster >= 0x0FFFFFF8){
        m_finished = true;
        return FSError::END_OF_DIRECTORY;
    }

    m_current_cluster = next_cluster;
    load_cluster();
    return FSError::SUCCESS;
}

FSError FAT32DirectoryIterator::next(FileEntry& file){
    if(m_finished){
        return FSError::END_OF_DIRECTORY;
    }
    
    DirectoryEntryFat32* entries = (DirectoryEntryFat32*)m_cluster_buffer;
    while(m_entry_index < m_entries_per_cluster){
        DirectoryEntryFat32* dir_entry = &entries[m_entry_index];
        m_entry_index++;
        if(dir_entry->name[0] == 0x00){
            m_finished = true;
            return FSError::END_OF_DIRECTORY;
        }
        if(dir_entry->name[0] == 0xE5){
            continue; // Deleted file
        }
        if((dir_entry->attributes & 0x0F) == 0x0F){
            continue; // LFN entry
        }
        convert_entry(dir_entry, file);
        return FSError::SUCCESS;
    }

    // If cluster has ended, load next and continue.
    FSError err = load_next_cluster();
    if(err != FSError::SUCCESS){
        return err;
    }

    return next(file);
}


void FAT32DirectoryIterator::convert_entry(DirectoryEntryFat32* src, FileEntry& dst){
    int name_len{};
    for(int i=0; i<8 && src->name[i]!=' '; ++i){ // Padded with spaces
        dst.name[name_len++] = src->name[i];
    }

    if(src->ext[0]!=' '){
        dst.name[name_len++] = '.';
        for(int i=0; i<3 && src->ext[i]!=' '; ++i){
            dst.name[name_len++] = src->ext[i];
        }
    }
    dst.name[name_len] = '\0';
    dst.size = src->size;
    dst.attributes = src->attributes;
    dst.first_cluster = ((uint32_t)src->first_cluster_hi << 16) | src->first_cluster_lo;
}

void FAT32DirectoryIterator::reset(){
    m_finished = false;
    m_entry_index = 0;
    m_current_cluster = m_start_cluster;
    load_cluster();
}

FAT32::FAT32(drivers::ATA* disk, uint32_t partition_offset)
: Filesystem(partition_offset), m_disk(disk)
{}

FAT32::~FAT32(){
    unmount();
}


FSError FAT32::mount(){
    m_disk->read_28(m_partition_offset, (uint8_t*)&m_bpb, sizeof(BiosParameterBlock32));
    
    if(m_bpb.bytes_per_sector != 512){
        return FSError::INVALID_FILESYSTEM;
    }

    m_fat_start = m_partition_offset + m_bpb.reserved_sectors;
    m_data_start = m_fat_start + m_bpb.table_size*m_bpb.fat_copies;

    m_mounted = true;
    return FSError::SUCCESS;
}


void FAT32::unmount(){
    m_mounted = false;
}


bool FAT32::detect(uint8_t* boot_sector){
    BiosParameterBlock32* bpb = (BiosParameterBlock32*)boot_sector;

    if(bpb->bytes_per_sector != 512) return false;
    if(bpb->fat_copies == 0) return false;
    if(bpb->table_size == 0) return false;

    const char label_name[9] = "FAT32   ";
    for(int i=0; i<8; ++i){
        if(bpb->fat_type_label[i] != label_name[i]){
            return false;
        }
    }

    return true;
}


FSError FAT32::open(const char* path, FileHandle& handle){
    if(!m_mounted) return FSError::INVALID_FILESYSTEM;

    FileEntry entry;
    FSError err = stat(path, entry);
    if(err != FSError::SUCCESS){
        return err;
    }

    if(entry.is_directory()){
        return FSError::NOT_A_FILE;
    }

    handle.cluster = entry.first_cluster;
    handle.start_cluster = entry.first_cluster;
    handle.position = 0;
    handle.size = entry.size;
    handle.valid = true;
    
    return FSError::SUCCESS;
}


FSError FAT32::read(FileHandle& handle, uint8_t* buffer, size_t size, uint32_t& bytes_read){
    if(!handle.valid) return FSError::INVALID_PATH;
    if(handle.position >= handle.size){
        bytes_read = 0; // Nothing to read
        return FSError::SUCCESS;
    }

    uint32_t remaining = handle.size - handle.position;
    if(size > remaining){
        size = remaining;
    }

    bytes_read = 0;
    const uint32_t CLUSTER_SIZE = m_bpb.bytes_per_sector * m_bpb.sectors_per_cluster;
    while(size > 0 && handle.cluster < 0x0FFFFFF8){
        uint32_t cluster_offset = handle.position % CLUSTER_SIZE;
        uint32_t cluster_bytes = CLUSTER_SIZE - cluster_offset;
        uint32_t to_read = (size < cluster_bytes) ? size : cluster_bytes;

        FSError err = read_cluster(handle.cluster, buffer + bytes_read, cluster_offset, to_read);
        if(err != FSError::SUCCESS){
            return err;
        }

        bytes_read += to_read;
        handle.position += to_read;
        size -= to_read;

        if (handle.position % CLUSTER_SIZE == 0 && size > 0) {
            handle.cluster = get_next_cluster(handle.cluster);
        }
    
    }

    return FSError::SUCCESS;
}


FSError FAT32::seek(FileHandle& handle, uint32_t position){
    if(!handle.valid) return FSError::INVALID_PATH;
    if(position > handle.size) return FSError::INVALID_PATH;
    // TODO Use more descriptor error types
    FileEntry entry;
    if(position==0){
        handle.cluster = handle.start_cluster;
        handle.position = 0;
    }
    else{
        // Only seek to start of file for now
        return FSError::INVALID_PATH;
    }
    // TODO: find which cluster `position` is in and set handle.cluster to it 

    return FSError::SUCCESS;
}


void FAT32::close(FileHandle& handle){
    handle.valid = false;
}


FSError FAT32::open_directory(const char* path, DirectoryIterator*& iterator){
    if(!m_mounted) return FSError::INVALID_FILESYSTEM;

    FileEntry entry;

    if(PathUtils::strcmp(path, "/")){ // Root dir
        entry.first_cluster = m_bpb.root_cluster;
        entry.attributes = ATTR_DIRECTORY;
    }
    else{
        FSError err = stat(path, entry);
        if(err != FSError::SUCCESS){
            return err;
        }
        if(!entry.is_directory()){
            return FSError::NOT_A_DIRECTORY;
        }
    }
    
    iterator = (FAT32DirectoryIterator*)memory_management::MemoryManager::active_memory_manager->malloc(sizeof(FAT32DirectoryIterator));
    if(iterator == 0){
        return FSError::OUT_OF_MEMORY;
    }
    new (iterator) FAT32DirectoryIterator(m_disk, &m_bpb, m_partition_offset, entry.first_cluster);

    return FSError::SUCCESS;
}


void FAT32::close_directory(DirectoryIterator* iterator){
    if(iterator){
        iterator->~DirectoryIterator();
        // memory_management::MemoryManager::active_memory_manager->free(iterator);
        ::operator delete(iterator);
    }
}


FSError FAT32::stat(const char* path, FileEntry& file){
    if(!m_mounted) return FSError::INVALID_FILESYSTEM;
    
    char components[16][256];
    uint32_t count;
    if(!PathUtils::split_path(path, components, 16, count)){
        return FSError::INVALID_PATH;
    }
    //Start from root
    uint32_t current_cluster = m_bpb.root_cluster;

    for(int i=0; i<count; ++i){
        bool found = false;

        FAT32DirectoryIterator iter(m_disk, &m_bpb, m_partition_offset, current_cluster);
        FileEntry temp_file;
        while(iter.next(temp_file) != FSError::END_OF_DIRECTORY){
            if(PathUtils::strcmp(temp_file.name, components[i])){
                if(i == count-1){
                    file = temp_file;
                    return FSError::SUCCESS;
                }
                else if(temp_file.is_directory()){
                    current_cluster = temp_file.first_cluster;
                    found = true;
                    break;
                }
                else{
                    return FSError::NOT_A_DIRECTORY;
                }
            }
            
        }
        if(!found && i < count-1){
            return FSError::NOT_FOUND;
        }
    }

    return FSError::NOT_FOUND;
}


bool FAT32::exists(const char* path){
    FileEntry temp;
    return stat(path, temp) == FSError::SUCCESS; 
}


const char* FAT32::get_fs_name() const {
    return "FAT32";
}



FSError FAT32::read_cluster(uint32_t cluster, uint8_t* buffer, uint32_t offset, uint32_t size){
    uint32_t cluster_sector = m_data_start + m_bpb.sectors_per_cluster * (cluster-2);
    uint32_t start_sector = cluster_sector + (offset / m_bpb.bytes_per_sector);
    uint32_t sector_offset = offset % m_bpb.bytes_per_sector;

    uint32_t bytes_read{};

    while(size > 0){
        uint32_t num_bytes = m_bpb.bytes_per_sector - sector_offset;
        if(num_bytes > size){
            num_bytes = size;
        }

        m_disk->read_28(start_sector, buffer + bytes_read, num_bytes);

        bytes_read += num_bytes;
        size -= bytes_read;
        start_sector++;
        sector_offset = 0;
    }

    return FSError::SUCCESS;
}


uint32_t FAT32::get_next_cluster(uint32_t cluster){
    uint32_t fat_sector = cluster / (m_bpb.bytes_per_sector / 4);
    uint32_t fat_offset = cluster % (m_bpb.bytes_per_sector / 4);

    m_disk->read_28(m_fat_start + fat_sector, m_sector_buf, m_bpb.bytes_per_sector);
    return ((uint32_t*)m_sector_buf)[fat_offset] & 0x0FFFFFFF;
}


