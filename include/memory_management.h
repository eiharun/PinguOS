#pragma once
#include <common/types.h>

using namespace common;

namespace memory_management {

struct MemoryNode{
    MemoryNode* next;
    MemoryNode* prev;
    bool allocated;
    size_t size;
};

class MemoryManager{
protected:
    MemoryNode* m_first;
public:
    static MemoryManager* active_memory_manager;
    MemoryManager(uint32_t start, size_t size);
    ~MemoryManager();

    void* malloc(size_t size);
    void free(void* ptr);
};


}