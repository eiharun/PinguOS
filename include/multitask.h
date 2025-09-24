#pragma once
#include <common/types.h>
#include <gdt.h>

using namespace common;

namespace multitasking {

    struct CPUState{
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;

        uint32_t esi;
        uint32_t edi;
        uint32_t ebp;

        uint32_t error;

        // Pushed by the processor
        uint32_t eip;
        uint32_t cs;
        uint32_t eflags;
        uint32_t esp;
        uint32_t ss;
    }__attribute__((packed));

    class Task{
    friend class TaskManager;
    private:
        uint8_t m_stack[4096];
        CPUState* m_cpu_state;
    public:
        Task(GlobalDescriptorTable* gdt, void entrypoint());
        ~Task();
    };

    class TaskManager{
    private:
        Task* m_tasks[256];
        int m_num_tasks;
        int m_curr_task;
    public:
        TaskManager();
        ~TaskManager();
        bool add_task(Task* task);
        CPUState* scheduler(CPUState* cpu_state);
    };



}