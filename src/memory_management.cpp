#include <memory_management.h>

using namespace memory_management;

MemoryManager* MemoryManager::active_memory_manager = 0;

MemoryManager::MemoryManager(uint32_t start, size_t size){
    active_memory_manager = this;

    if(size < sizeof(MemoryNode)){
        m_first = 0;
    }
    else{
        m_first = (MemoryNode*)start;
        m_first->allocated = false;
        m_first->next = 0;
        m_first->prev = 0;
        m_first->size = size - sizeof(MemoryNode);
    }

}

MemoryManager::~MemoryManager(){
    if(active_memory_manager == this){
        active_memory_manager = 0;
    }
}

/* Not a very efficient approach, but it works for now */
void* MemoryManager::malloc(size_t size, size_t alignment /*=0*/){
    if(alignment == 0 || alignment == 1) {
        alignment = 1; // treat "no alignment" as byte-aligned
    }
    MemoryNode* result = 0;
    for(MemoryNode* chunk=m_first; (chunk != 0 && result == 0); chunk = chunk->next){
        if(size < chunk->size && !chunk->allocated){
            result = chunk;
        }
    }
    if(result == 0){
        return 0;
    }

    uint32_t raw_addr = (uint32_t)result + sizeof(MemoryNode);

    // round up to alignment boundary
    uint32_t aligned_addr = (raw_addr + (alignment - 1)) & ~(alignment - 1);

    // padding between raw and aligned
    uint8_t padding = (uint8_t)(aligned_addr - raw_addr);

    if(result->size >= size + padding + sizeof(MemoryNode) + 1){
        MemoryNode* temp = (MemoryNode*)(aligned_addr + size);
        temp->allocated = false;
        temp->size = result->size - size - padding - sizeof(MemoryNode); 
        temp->prev = result;
        temp->next = result->next; // Temp->next is whatever was result->next previously
        if(temp->next != 0){
            temp->next->prev = temp; // (temp)/result->next->prev should be temp
        }
        
        result->size = size + padding;
        result->next = temp;

    }
    result->allocated = true;
    result->offset = padding;
    return (void*)aligned_addr;
}

void MemoryManager::free(void* ptr){
    if(ptr == 0) return;

    MemoryNode* to_be_freed = (MemoryNode*)((uint32_t)ptr - sizeof(MemoryNode));
    to_be_freed = (MemoryNode*)((uint32_t)to_be_freed - to_be_freed->offset);
    to_be_freed->allocated = false;
    // Attempt to merge prev
    if(to_be_freed->prev !=0 && to_be_freed->prev->allocated == false){
        to_be_freed->prev->next = to_be_freed->next;
        to_be_freed->prev->size += to_be_freed->size + sizeof(MemoryNode); //resize
        if(to_be_freed->next != 0){
            to_be_freed->next->prev = to_be_freed->prev;
        }
        to_be_freed = to_be_freed->prev;
    }
    // Attmept to merge next
    if(to_be_freed->next !=0 && to_be_freed->next->allocated == false){
        to_be_freed->size += to_be_freed->next->size + sizeof(MemoryNode);
        to_be_freed->next = to_be_freed->next->next;
        if(to_be_freed->next != 0){
            to_be_freed->next->prev = to_be_freed;
        }
    }
}

void* operator new(size_t size){
    if(MemoryManager::active_memory_manager == 0){
        return 0;
    }
    return MemoryManager::active_memory_manager->malloc(size);
}
void* operator new[](size_t size){
    if(MemoryManager::active_memory_manager == 0){
        return 0;
    }
    return MemoryManager::active_memory_manager->malloc(size);
}
void* operator new(size_t size, void* ptr){
   return ptr;
}
void* operator new[](size_t size, void* ptr){
    return ptr;
}

void operator delete(void* ptr, uint32_t size){
    if(MemoryManager::active_memory_manager != 0){
        MemoryManager::active_memory_manager->free(ptr);
    }
}
void operator delete(void* ptr){
    if(MemoryManager::active_memory_manager != 0){
        MemoryManager::active_memory_manager->free(ptr);
    }
}

void operator delete[](void* ptr){
    if(MemoryManager::active_memory_manager != 0){
        MemoryManager::active_memory_manager->free(ptr);
    }
}

