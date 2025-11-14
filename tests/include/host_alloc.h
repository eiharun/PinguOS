#include <allocator.h>
#include <cstdlib>

using namespace memory_management;

class HostAllocator: public Allocator{
public:
    HostAllocator();

    void* malloc(size_t size, size_t alignment=0);

    void free(void* ptr);

};

extern HostAllocator memory_manager;


