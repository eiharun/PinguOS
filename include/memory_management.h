#pragma once
#include <common/types.h>
#include <allocator.h>

using namespace common;

namespace memory_management {
// #ifdef KERNEL_BUILD

struct MemoryNode{
    MemoryNode* next;
    MemoryNode* prev;
    bool allocated;
    size_t size;
    uint8_t offset;
};

class LinearAllocator: public Allocator{
protected:
    MemoryNode* m_first;
public:
    // static MemoryManager* active_memory_manager;
    LinearAllocator(uint32_t start, size_t size);
    ~LinearAllocator();

    void* malloc(size_t size, size_t alignment=0) override;
    void free(void* ptr) override;
};

// #endif
}

