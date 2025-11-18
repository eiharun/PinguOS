#include <allocator.h>
#include <cstdlib>
#include <memory>
#include <unordered_map>

typedef unsigned char uint8_t;

using namespace memory_management;

class HostAllocator: public Allocator{
public:
    HostAllocator();

    void* malloc(size_t size, size_t alignment=0);

    void free(void* ptr);
private:
    std::unordered_map<void*, std::unique_ptr<uint8_t[]>> allocations;
};

extern HostAllocator memory_manager;


