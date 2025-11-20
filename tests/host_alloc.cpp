#include "include/host_alloc.h"
#include <memory>

HostAllocator memory_manager;

HostAllocator::HostAllocator(){
    Allocator::active_memory_manager = this;
}

void* HostAllocator::malloc(size_t size, size_t alignment)  {
    
    return std::malloc(size == 0 ? 1 : size);
}

void HostAllocator::free(void* ptr) {
    std::free(ptr);
}

// void* operator new(size_t size){
//     if(Allocator::active_memory_manager == 0){
//         return 0;
//     }
//     return Allocator::active_memory_manager->malloc(size);
// }
// void* operator new[](size_t size){
//     if(Allocator::active_memory_manager == 0){
//         return 0;
//     }
//     return Allocator::active_memory_manager->malloc(size);
// }
// void* operator new(size_t size, void* ptr) noexcept {
//    return ptr;
// }
// void* operator new[](size_t size, void* ptr) noexcept {
//     return ptr;
// }

// void operator delete(void* ptr, size_t size){
//     if(Allocator::active_memory_manager != 0){
//         Allocator::active_memory_manager->free(ptr);
//     }
// }
// void operator delete(void* ptr){
//     if(Allocator::active_memory_manager != 0){
//         Allocator::active_memory_manager->free(ptr);
//     }
// }

// void operator delete[](void* ptr){
//     if(Allocator::active_memory_manager != 0){
//         Allocator::active_memory_manager->free(ptr);
//     }
// }