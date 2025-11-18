#pragma once

typedef unsigned int uint32_t; 
typedef uint32_t size_t;


namespace memory_management{

class Allocator{
public:
    virtual ~Allocator() = default;
    static Allocator* active_memory_manager;
    virtual void* malloc(size_t size, size_t alignment=0) = 0;
    virtual void free(void* ptr) = 0;
};

}

// #ifdef KERNEL_BUILD
void* operator new(size_t size);
void* operator new[](size_t size);

void* operator new(size_t size, void* ptr) noexcept;
void* operator new[](size_t size, void* ptr) noexcept;

void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;
// #endif