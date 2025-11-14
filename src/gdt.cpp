#include <gdt.h>

// Fail compilation if not 32-bit mode
#if __SIZEOF_POINTER__ != 4
#error "This code must be compiled in 32-bit mode."
#endif

GlobalDescriptorTable::GlobalDescriptorTable()
:m_nullSegment(0,0,0), m_unusedSegment(0,0,0), m_codeSegment(0,64*1024*1024,0x9A), m_dataSegment(0,64*1024*1024,0x92)
{
    GDTPointer gdt;
    gdt.limit = sizeof(GlobalDescriptorTable)-1;
    gdt.base = (uint32_t)this;
    // uint32_t i[2];
    // i[0] = (uint32_t)this;
    // i[1] = sizeof(GlobalDescriptorTable) << 16;
    // asm volatile("lgdt (%0)": :"p" (((uint8_t *) i)+2));
    asm volatile("lgdt %0" : : "m"(gdt));
    
}

GlobalDescriptorTable::~GlobalDescriptorTable(){}

uint16_t GlobalDescriptorTable::dataSegmentSelector(){
    return ((uint8_t*)&m_dataSegment - (uint8_t*)this);
}

uint16_t GlobalDescriptorTable::codeSegmentSelector(){
    return ((uint8_t*)&m_codeSegment - (uint8_t*)this);
}

GlobalDescriptorTable::SegmentDescriptor::SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t flags){
    uint8_t* target = (uint8_t*)this; // cast class to an array of bytes

    if(limit <= (1 << 16)){ // 2^16 = 65536 (64 KB)
        target[6] = 0x40; // flag/limit_hi byte (set G bit to 0)
    }
    else{
        if((limit & 0xFFF) != 0xFFF){
            limit = (limit >> 12) -1;
        }
        else{
            limit = limit >> 12;
        }
        target[6] = 0xC0; // flag/limit_hi byte (set G bit to 1)
    }

    target[0] = limit & 0xFF; //set limit
    target[1] = (limit>>8) & 0xFF;

    target[2] = base & 0xFF; //set base ptr
    target[3] = (base>>8) & 0xFF;
    target[4] = (base>>16) & 0xFF;
    target[7] = (base>>24) & 0xFF;
    
    target[5] = flags;
    target[6] |= (limit >> 16) & 0xF; //set limit_hi

}

uint32_t GlobalDescriptorTable::SegmentDescriptor::Base(){
    uint8_t* target = (uint8_t*)this;
    uint32_t result = (target[7]<<24);
    result = result + (target[4]<<16) + (target[3]<<8) + (target[2]);
    return result;
}


uint32_t GlobalDescriptorTable::SegmentDescriptor::Limit(){
    uint8_t* target = (uint8_t*)this;
    uint32_t result = (target[6] & 0xF)<<16; //low 4 bits
    result = result + (target[1]<<8) + target[0];
    if((target[6] & 0xC0) == 0xC0){
        result = (result<<12) & 0xFFF;
    }
    return result;
}