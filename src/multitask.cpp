#include <multitask.h>

using namespace multitasking;

Task::Task(GlobalDescriptorTable* gdt, void entrypoint()){
    m_cpu_state = (CPUState*)(m_stack + 4096 - sizeof(CPUState));

    m_cpu_state->eax = 0;
    m_cpu_state->ebx = 0;
    m_cpu_state->ecx = 0;
    m_cpu_state->edx = 0;

    m_cpu_state->esi = 0;
    m_cpu_state->edi = 0;
    m_cpu_state->ebp = 0;

    // m_cpu_state->error = ;

    m_cpu_state->eip = (uint32_t)entrypoint;
    m_cpu_state->cs = (uint32_t)gdt->codeSegmentSelector();
    m_cpu_state->eflags = 0x202;
    // m_cpu_state->esp = ;
    // m_cpu_state->ss = ;

}

Task::~Task(){}


TaskManager::TaskManager()
: m_num_tasks(0), m_curr_task(-1){

}

TaskManager::~TaskManager(){}

bool TaskManager::add_task(Task* task){
    if(m_num_tasks >= 256){
        return false;
    }
    m_tasks[m_num_tasks++] = task;
    return true;
}

CPUState* TaskManager::scheduler(CPUState* cpu_state){
    if(m_num_tasks <= 0){
        return cpu_state;
    }
    if(m_curr_task >= 0){
        m_tasks[m_curr_task]->m_cpu_state = cpu_state;
    }
    if(++m_curr_task >= m_num_tasks){
        m_curr_task %= m_num_tasks;
    }
    return m_tasks[m_curr_task]->m_cpu_state;
}
