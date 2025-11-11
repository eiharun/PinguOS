#pragma once
#include <common/types.h>

using namespace common;

namespace filesystem{

enum class FSError{
    SUCCESS = 0,
    NOT_FOUND,
    INVALID_PATH,
    NOT_A_DIRECTORY,
    NOT_A_FILE,
    END_OF_DIRECTORY,
    READ_ERROR,
    DISK_ERROR,
    INVALID_FILESYSTEM,
    BUFFER_TOO_SMALL,
    OUT_OF_MEMORY,
    DISK_FULL,
    END_OF_CHAIN
};

enum FileAttributes : uint8_t {
    ATTR_READ_ONLY  = 0x01,
    ATTR_HIDDEN     = 0x02,
    ATTR_SYSTEM     = 0x04,
    ATTR_VOLUME_ID  = 0x08,
    ATTR_DIRECTORY  = 0x10,
    ATTR_ARCHIVE    = 0x20
};

struct FileEntry {
    char name[256];           // Filename (null-terminated)
    uint32_t size;            // File size in bytes
    uint8_t attributes;       // File attributes bitfield
    uint32_t first_cluster;   // First cluster (filesystem-specific)
    uint32_t parent_cluster;  // Parent cluster
    
    // Helper methods
    bool is_directory() const { return attributes & ATTR_DIRECTORY; }
    bool is_file() const { return !(attributes & ATTR_DIRECTORY); }
    bool is_hidden() const { return attributes & ATTR_HIDDEN; }
};

struct FileHandle {
    uint32_t cluster;         // Current cluster
    uint32_t start_cluster;   // Start cluster
    uint32_t parent_cluster;  // Parent cluster
    uint32_t position;        // Current byte position in file
    uint32_t size;            // Total file size
    void* fs_specific;        // Filesystem-specific data
    bool valid;
    
    FileHandle() : cluster(0), position(0), size(0), fs_specific(0), valid(false) {}
};

class DirectoryIterator{
protected:
    uint32_t m_cur_cluster;
    uint32_t m_cluster_offset;
    bool m_finished;
    void* m_fs;
public:
    DirectoryIterator(): m_cur_cluster(0), m_cluster_offset(0), 
        m_finished(true), m_fs(0) {}
    virtual ~DirectoryIterator(){}
    virtual FSError next(FileEntry& file) = 0;
    virtual bool is_finished() const { return m_finished; }
    virtual void reset() = 0;
};


class Filesystem{
protected:
    uint32_t m_partition_offset;
    bool m_mounted;
public:
    Filesystem(uint32_t partition_offset): m_partition_offset(partition_offset), m_mounted(false) {}
    virtual ~Filesystem(){}
    virtual FSError mount() = 0;
    virtual void unmount() = 0;
    
    static bool detect(uint8_t* boot_sector) { return false; }
    bool is_mounted() const { return m_mounted; }
    
    enum WRITE_MODE{WRITE=0, APPEND};

    virtual FSError open(const char* path, FileHandle& handle) = 0;
    virtual FSError write(FileHandle& handle, uint8_t* buffer, size_t size, WRITE_MODE mode) = 0;
    virtual FSError read(FileHandle& handle, uint8_t* buffer, size_t size, uint32_t& bytes_read) = 0;
    virtual FSError seek(FileHandle& handle, uint32_t position) = 0;
    virtual void close(FileHandle& handle) = 0;

    virtual FSError make_file(const char* path, const char* filename) = 0;
    virtual FSError delete_file(FileHandle& handle) = 0;
    virtual FSError make_directory(const char* path, const char* dirname) = 0;
    virtual FSError delete_directory(const char* path) = 0;
    
    virtual FSError open_directory(const char* path, DirectoryIterator*& iterator) = 0;
    virtual void close_directory(DirectoryIterator* iterator) = 0;
    virtual FSError stat(const char* path, FileEntry& file) = 0;
    virtual bool exists(const char* path) = 0;
    virtual const char* get_fs_name() const = 0;
};

}