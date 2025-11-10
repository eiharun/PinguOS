#include "drivers/intel_82540em.h"
#include "filesystem/filesystem.h"
#include <filesystem/fat.h>
#include <memory_management.h>
#include <common/path_utils.hpp>

using namespace filesystem;

void printf(char*);
void printf_hex(uint8_t);
void printf_hex16(uint16_t);
void printf_hex32(uint32_t);

//========---FAT32DirectoryIterator START---========

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
    // Index by number of current cluster, increments of sectors
    for(int i=0; i<m_bpb->sectors_per_cluster; ++i){
        m_disk->read_28(cluster_sector+i, m_cluster_buffer+(m_bpb->bytes_per_sector*i), m_bpb->bytes_per_sector);
    }

    m_entry_index = 0;
}

FSError FAT32DirectoryIterator::load_next_cluster(){
    uint32_t fat_start = m_partition_offset + m_bpb->reserved_sectors;
    uint32_t fat_sector = m_current_cluster / (m_bpb->bytes_per_sector / 4); // Find sector within FAT where current cluster is
    uint32_t fat_offset = m_current_cluster % (m_bpb->bytes_per_sector / 4); // Find offset within the FAT sector where the cluster is

    uint8_t fat_buf[512];
    m_disk->read_28(fat_start+fat_sector, fat_buf, m_bpb->bytes_per_sector);
    uint32_t next_cluster = ((uint32_t*)fat_buf)[fat_offset] & 0x0FFFFFFF;
    // Read next cluster from FAT

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
    dst.parent_cluster = m_current_cluster;
}

void FAT32DirectoryIterator::reset(){
    m_finished = false;
    m_entry_index = 0;
    m_current_cluster = m_start_cluster;
    load_cluster();
}   
//========---FAT32DirectoryIterator END-----========


//========---FAT32 START---========

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
    if(m_bpb.sectors_per_cluster > 8){
        // DirIterator static cluster buffer max size supports 8 sectors
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
    handle.parent_cluster = entry.parent_cluster;
    handle.position = 0;
    handle.size = entry.size;
    handle.valid = true;
    
    return FSError::SUCCESS;
}

FSError FAT32::write(FileHandle& handle, uint8_t* buffer, size_t size, WRITE_MODE mode){
    if(!handle.valid) return FSError::INVALID_PATH;
    

    switch(mode){
    case WRITE_MODE::APPEND:{
        // Shift *size* number of bytes *size* bytes at handle.position in cluster and following clusters
    } break;
    case WRITE_MODE::WRITE:{
        // Overwrite at handle.position
    } break;
    }

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


FSError FAT32::make_file(const char* path, const char* filename){
    // TODO: Disallow duplicate files in same dir
    FileEntry entry;
    uint32_t parent_cluster;
    if(PathUtils::strcmp(path, "/")){
        parent_cluster = m_bpb.root_cluster;
    }
    else{
        FSError err = stat(path, entry);
        parent_cluster = entry.parent_cluster;
    }
    // Allocate cluster & update FAT
    uint32_t file_cluster = allocate_cluster();
    if(file_cluster == 0){
        return FSError::DISK_FULL;
    }
    // Clear cluster (0)
    zero_cluster(file_cluster);
    // Add dir entry to parent directory
    FileEntry new_file_entry;
    int name_len{};
    for(int i=0; filename[i]!='\0'; ++i){
        new_file_entry.name[name_len++] = filename[i];
    }
    new_file_entry.name[name_len] = '\0';
    new_file_entry.parent_cluster = parent_cluster;
    new_file_entry.size = 0;
    new_file_entry.attributes = ATTR_ARCHIVE;
    new_file_entry.first_cluster = file_cluster;
    create_directory_entry(new_file_entry);

    return FSError::SUCCESS;
}

FSError FAT32::delete_file(FileHandle& handle){

}

FSError FAT32::make_directory(const char* path, const char* dirname){

}

FSError FAT32::delete_directory(const char* path){

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

FSError FAT32::print_list_directory(void(*printf)(char*), const char* path){
    DirectoryIterator* iter = nullptr;
    FSError err = open_directory("/", iter);
    if(err != FSError::SUCCESS){
        printf("Failed to open root dir\n");
        return err;
    }
    FileEntry entry;
    while(iter->next(entry) == FSError::SUCCESS){
        printf(" ");
        if(entry.is_directory()){
            printf("[DIR] ");
        }
        else{
            printf("[FILE] ");
        }
        printf(entry.name);
        if(entry.is_file()){
            printf(" (0x");
            printf_hex32(entry.size);
            printf(" bytes) ");
        }
        printf("\n");
    }
    close_directory(iter);
    return FSError::SUCCESS;
}


//========-FAT32 PRIV FUNC-========

FSError FAT32::read_cluster(uint32_t cluster, uint8_t* buffer, uint32_t offset, uint32_t size){
    uint32_t cluster_sector = m_data_start + m_bpb.sectors_per_cluster * (cluster-2);
    uint32_t start_sector = cluster_sector + (offset / m_bpb.bytes_per_sector);
    uint32_t sector_offset = offset % m_bpb.bytes_per_sector;

    uint32_t bytes_read{};

    // Clamp size to cluster size
    if(size > m_bpb.sectors_per_cluster*m_bpb.bytes_per_sector){
        size = m_bpb.sectors_per_cluster*m_bpb.bytes_per_sector;
    }

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

FSError FAT32::write_cluster(uint32_t cluster, uint8_t* buffer, uint32_t offset, uint32_t size){
    uint32_t cluster_sector = m_data_start + m_bpb.sectors_per_cluster * (cluster-2);
    uint32_t start_sector = cluster_sector + (offset / m_bpb.bytes_per_sector);
    uint32_t sector_offset = offset % m_bpb.bytes_per_sector;
    
    uint32_t bytes_written{};

    // Clamp size to cluster size
    if(size > m_bpb.sectors_per_cluster*m_bpb.bytes_per_sector){
        size = m_bpb.sectors_per_cluster*m_bpb.bytes_per_sector;
    }

    while(size > 0){
        uint32_t num_bytes = m_bpb.bytes_per_sector - sector_offset;
        if(num_bytes > size){
            num_bytes = size;
        }

        m_disk->write_28(start_sector, buffer + bytes_written, num_bytes);

        bytes_written += num_bytes;
        size -= bytes_written;
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

void FAT32::link_next_cluster(uint32_t current_cluster, uint32_t next_cluster){
    uint32_t fat_sector = current_cluster / (m_bpb.bytes_per_sector / 4);
    uint32_t fat_offset = current_cluster % (m_bpb.bytes_per_sector / 4);

    m_disk->read_28(m_fat_start + fat_sector, m_sector_buf, m_bpb.bytes_per_sector);
    ((uint32_t*)m_sector_buf)[fat_offset] = next_cluster;
    m_disk->write_28(m_fat_start + fat_sector, m_sector_buf, m_bpb.bytes_per_sector);
    zero_cluster(next_cluster); //FIXME: Is this necesarry?
}

FSError FAT32::zero_cluster(uint32_t cluster){
    uint8_t zero[1] {0};
    for(int i=0; i<m_bpb.sectors_per_cluster; ++i){
        // write_28 will always fill the rest of the sector with 0
        m_disk->write_28(m_data_start+(m_bpb.sectors_per_cluster*(cluster-2))+i, zero, 1);
    }

}

uint32_t FAT32::allocate_cluster(){
    uint32_t cluster{};
    const size_t NUM_ENTRIES = m_bpb.table_size*(m_bpb.bytes_per_sector/4);
    uint8_t* sector = (uint8_t*)memory_management::MemoryManager::active_memory_manager->malloc(m_bpb.bytes_per_sector);
    for(uint32_t cluster = 2; cluster<=NUM_ENTRIES; ++cluster){
        uint32_t fat_sector = cluster / (m_bpb.bytes_per_sector/4);
        uint32_t fat_offset = cluster % (m_bpb.bytes_per_sector/4);
        m_disk->read_28(m_fat_start+fat_sector, sector, m_bpb.bytes_per_sector);
        uint32_t* entries = (uint32_t*)sector;
        if((entries[fat_offset] & 0x0FFFFFFF) == 0){
            entries[fat_offset] = 0x0FFFFFFF;
            m_disk->write_28(m_fat_start+fat_sector, (uint8_t*)entries, m_bpb.bytes_per_sector);
            return cluster;
        }
    }    
    memory_management::MemoryManager::active_memory_manager->free(sector);
    return 0;
}

FSError FAT32::create_directory_entry(FileEntry& new_entry){
    FSError err{FSError::SUCCESS};
    // TODO: After implementing date-time, fill in c/a/w date/time
    uint32_t curr_cluster = new_entry.parent_cluster;
    bool written{false};
    bool need_alloc{false};
    const size_t CLUSTER_SIZE = m_bpb.sectors_per_cluster*m_bpb.bytes_per_sector;
    uint8_t* cluster_buf = (uint8_t*)memory_management::MemoryManager::active_memory_manager->malloc(CLUSTER_SIZE);
    while(!written){
        // Read parent cluster for dir entries.
        read_cluster(curr_cluster, cluster_buf, 0, CLUSTER_SIZE);
        // Look for empty space (name[0] == 0 or 0xE5)
        DirectoryEntryFat32* entries = (DirectoryEntryFat32*)cluster_buf;
        const int NUM_ENTRIES = CLUSTER_SIZE / sizeof(DirectoryEntryFat32);
        uint32_t written_entry_offset{};
        for(int i=0; i<NUM_ENTRIES; ++i){
            if((entries[i].name[0] == ENTRY_END) || (entries[i].name[0] == ENTRY_DELETED)){
                //Write to entries[i]
                // Copy 8.3 standard name 
                bool dot{false};
                int name_len{};
                int ext_len{};
                for(int s=0; new_entry.name[s]!='\0'; ++s){
                    if(new_entry.name[s] == '.'){
                        dot = true;
                    }
                    if(!dot){
                        // Copy name
                        if(s<=8){
                            entries[i].name[s] = new_entry.name[s];
                            name_len++;
                        }
                    }
                    else{
                        // Pad name if necesarry
                        if(name_len < 8){
                            for(int j=name_len; j<8; ++j){
                                entries[i].name[j] = ' ';
                            }
                            name_len = 8;
                        }
                        // Copy ext
                        if(entries[i].name[s] != '.'){
                            entries[i].ext[s] = new_entry.name[s];
                            ext_len++;
                        }
                    }
                }
                // Pad ext if necesarry
                if(ext_len<3){
                    for(int j=ext_len; j<3; ++j){
                        entries[i].ext[j] = ' ';
                    }
                }

                entries[i].attributes = new_entry.attributes;
                entries[i].size = new_entry.size;
                entries[i].first_cluster_hi = (new_entry.first_cluster >> 16) & 0xFFFF;
                entries[i].first_cluster_lo = new_entry.first_cluster & 0xFFFF;
                
                written = true;
                written_entry_offset = i*sizeof(DirectoryEntryFat32);
                break;
            }
        }

        if(!written){
            // If not found, follow fat chain to next cluster
            // Look again
            uint32_t next_cluster = get_next_cluster(curr_cluster);
            // If no space at all (next cluster == END OF CHAIN)
            if(IS_END_OF_CHAIN(next_cluster)){
                need_alloc = true;
            }
            else{
                curr_cluster = next_cluster;
            }
        }
        
        if(need_alloc){
            // Allocate cluster
            uint32_t next_cluster = allocate_cluster();
            if(next_cluster == 0){
                err = FSError::DISK_FULL;
                break;
            } 
            // Link to fat
            curr_cluster = next_cluster;
            link_next_cluster(curr_cluster, next_cluster);
            need_alloc = false;
        }
        if(written){
            // Write dir entry into new cluster
            // TODO: Identify which sector entry is in and only write back that one for performance
            write_cluster(curr_cluster, cluster_buf, 0, CLUSTER_SIZE);
        }
    }
    memory_management::MemoryManager::active_memory_manager->free(cluster_buf);
    return err;
}

//========---FAT32 END-----========
