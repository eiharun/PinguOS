#pragma once
#include <common/types.h>

using namespace common;

class GlobalDescriptorTable{
public:
    struct GDTPointer{
        uint16_t limit;
        uint32_t base;
    }__attribute__((packed)); 

    class SegmentDescriptor{
    private:
        uint16_t limit_lo;
        uint16_t base_lo;
        uint8_t base_mid;
        uint8_t access;
        uint8_t flag_limit_hi;
        uint8_t base_hi;
    public:
        SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t access);
        uint32_t Base();
        uint32_t Limit();
    }__attribute__((packed)); // bytes aligned as above

    SegmentDescriptor m_nullSegment;
    SegmentDescriptor m_unusedSegment;
    SegmentDescriptor m_codeSegment;
    SegmentDescriptor m_dataSegment;

    GlobalDescriptorTable();
    ~GlobalDescriptorTable();

    uint16_t codeSegmentSelector();
    uint16_t dataSegmentSelector();
};

