#include <allocator.h>
#include <cstdlib>

typedef unsigned char uint8_t;

using namespace memory_management;

class HostAllocator: public Allocator{
public:
    HostAllocator();

    void* malloc(size_t size, size_t alignment=0);

    void free(void* ptr);
private:
};

extern HostAllocator memory_manager;


