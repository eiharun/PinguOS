#pragma once
#include <common/types.h>

using namespace common;

namespace memory_management {

struct MemoryNode{
    MemoryNode* next;
    MemoryNode* prev;
    bool allocated;
    size_t size;
    uint8_t offset;
};

class MemoryManager{
protected:
    MemoryNode* m_first;
public:
    static MemoryManager* active_memory_manager;
    MemoryManager(uint32_t start, size_t size);
    ~MemoryManager();

    void* malloc(size_t size, size_t alignment=0);
    void free(void* ptr);
};

}

void* operator new(size_t size);
void* operator new[](size_t size);

// placement new
void* operator new(size_t size, void* ptr);
void* operator new[](size_t size, void* ptr);

void operator delete(void* ptr);
void operator delete[](void* ptr);