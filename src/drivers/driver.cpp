#include <drivers/driver.h>

Driver::Driver(){}

Driver::~Driver(){}

void Driver::activate(){}

int Driver::reset(){
    return 0;
}

void Driver::deactivate(){}


DriverManager::DriverManager(){
    m_num_drivers = 0;
}

void DriverManager::add_driver(Driver* driver){
    if(m_num_drivers < 255){
        m_drivers[m_num_drivers] = driver;
        m_num_drivers++;
    }
}
void DriverManager::activate_all(){
    for(int i=0; i<m_num_drivers; ++i){
        m_drivers[i]->activate();
    }
}